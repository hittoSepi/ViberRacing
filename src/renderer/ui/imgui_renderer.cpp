#include "imgui_renderer.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <bgfx/bgfx.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <vector>

namespace viber {

static std::string findFontPath() {
    const char* candidates[] = {
        "assets/fonts/MesloLGS NF Regular.ttf",
        "../assets/fonts/MesloLGS NF Regular.ttf",
        "../../assets/fonts/MesloLGS NF Regular.ttf",
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) return path;
    }
    return "assets/fonts/MesloLGS NF Regular.ttf";
}

static std::string findShaderPath(const char* name) {
    std::vector<std::string> candidates = {
        std::string("assets/shaders/") + name,
        std::string("../assets/shaders/") + name,
        std::string("../../assets/shaders/") + name,
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) return path;
    }
    return candidates[0];
}

ImGuiRenderer::ImGuiRenderer() = default;

ImGuiRenderer::~ImGuiRenderer() {
    shutdown();
}

void ImGuiRenderer::init(GLFWwindow* window) {
    if (m_initialized) return;
    
    m_window = window;
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Load custom font
    io.Fonts->Clear();
    std::string fontPath = findFontPath();
    spdlog::info("Looking for font at: {}", fontPath);
    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);
    if (io.Fonts->Fonts.empty()) {
        spdlog::warn("Failed to load font: {}, using default", fontPath);
        io.Fonts->AddFontDefault();
    } else {
        spdlog::info("Loaded font: {}", fontPath);
    }
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Requires docking branch
    
    setupStyle();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    
    m_layout.begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
    
    m_u_texture = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);
    
    // Build font atlas
    io.Fonts->Build();
    
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    
    spdlog::info("Font atlas size: {}x{}, pixels: {}", width, height, pixels ? "valid" : "null");
    
    // Create texture with proper flags
    m_fontTexture = bgfx::createTexture2D(
        static_cast<u16>(width), static_cast<u16>(height),
        false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT,
        bgfx::copy(pixels, width * height * 4)
    );
    
    // Store texture handle as ImTextureID (pointer to handle)
    static bgfx::TextureHandle fontHandle;
    fontHandle = m_fontTexture;
    io.Fonts->TexID = reinterpret_cast<ImTextureID>(&fontHandle);
    
    spdlog::info("Font texture created: idx={}", m_fontTexture.idx);
    
    // Create shader program for ImGui
    m_program = createImGuiShader();
    if (!bgfx::isValid(m_program)) {
        spdlog::error("Failed to create ImGui shader program!");
    }
    
    m_initialized = true;
    spdlog::info("ImGui initialized");
}

bgfx::ProgramHandle ImGuiRenderer::createImGuiShader() {
    // Try to load from file first
    std::string vsPath = findShaderPath("vs_textured.bin");
    std::string fsPath = findShaderPath("fs_textured.bin");
    spdlog::info("Loading ImGui shaders: {} and {}", vsPath, fsPath);
    bgfx::ShaderHandle vs = loadShaderFromFile(vsPath.c_str());
    bgfx::ShaderHandle fs = loadShaderFromFile(fsPath.c_str());
    
    if (!bgfx::isValid(vs) || !bgfx::isValid(fs)) {
        spdlog::error("Failed to load ImGui shaders!");
        return BGFX_INVALID_HANDLE;
    }
    
    spdlog::info("Creating ImGui shader program");
    return bgfx::createProgram(vs, fs, true);
}

bgfx::ShaderHandle ImGuiRenderer::loadShaderFromFile(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        spdlog::warn("Shader file not found: {}", path);
        return BGFX_INVALID_HANDLE;
    }
    
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    const bgfx::Memory* mem = bgfx::copy(data.data(), static_cast<uint32_t>(size));
    bgfx::ShaderHandle handle = bgfx::createShader(mem);
    
    if (!bgfx::isValid(handle)) {
        spdlog::error("Failed to create shader from: {}", path);
    }
    
    return handle;
}

void ImGuiRenderer::shutdown() {
    if (!m_initialized) return;
    
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    if (bgfx::isValid(m_program)) bgfx::destroy(m_program);
    if (bgfx::isValid(m_u_texture)) bgfx::destroy(m_u_texture);
    if (bgfx::isValid(m_fontTexture)) bgfx::destroy(m_fontTexture);
    if (bgfx::isValid(m_vbh)) bgfx::destroy(m_vbh);
    if (bgfx::isValid(m_ibh)) bgfx::destroy(m_ibh);
    
    m_initialized = false;
}

void ImGuiRenderer::beginFrame() {
    if (!m_initialized) return;
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiRenderer::endFrame() {
    if (!m_initialized) return;
    ImGui::Render();
}

void ImGuiRenderer::render() {
    if (!m_initialized) return;
    
    ImDrawData* drawData = ImGui::GetDrawData();
    if (!drawData || drawData->CmdListsCount == 0) return;
    
    const ImGuiIO& io = ImGui::GetIO();
    const float width = io.DisplaySize.x;
    const float height = io.DisplaySize.y;
    
    bgfx::setViewName(255, "ImGui");
    bgfx::setViewMode(255, bgfx::ViewMode::Sequential);
    
    const bgfx::Caps* caps = bgfx::getCaps();
    {
        float ortho[16];
        // Simple ortho projection matrix
        const float left = 0.0f, right = width, bottom = height, top = 0.0f, near = 0.0f, far = 1000.0f;
        ortho[0] = 2.0f / (right - left); ortho[1] = 0.0f; ortho[2] = 0.0f; ortho[3] = 0.0f;
        ortho[4] = 0.0f; ortho[5] = 2.0f / (top - bottom); ortho[6] = 0.0f; ortho[7] = 0.0f;
        ortho[8] = 0.0f; ortho[9] = 0.0f; ortho[10] = 1.0f / (far - near); ortho[11] = 0.0f;
        ortho[12] = -(right + left) / (right - left);
        ortho[13] = -(top + bottom) / (top - bottom);
        ortho[14] = -near / (far - near); ortho[15] = 1.0f;
        bgfx::setViewTransform(255, nullptr, ortho);
        bgfx::setViewRect(255, 0, 0, static_cast<u16>(width), static_cast<u16>(height));
    }
    
    for (int n = 0; n < drawData->CmdListsCount; ++n) {
        const ImDrawList* cmdList = drawData->CmdLists[n];

        const u32 numVertices = static_cast<u32>(cmdList->VtxBuffer.Size);
        const u32 numIndices  = static_cast<u32>(cmdList->IdxBuffer.Size);
        const bool idx32 = sizeof(ImDrawIdx) == 4;

        if (bgfx::getAvailTransientVertexBuffer(numVertices, m_layout) < numVertices ||
            bgfx::getAvailTransientIndexBuffer(numIndices, idx32) < numIndices) {
            continue;
        }

        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer  tib;
        bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_layout);
        bgfx::allocTransientIndexBuffer(&tib, numIndices, idx32);
        std::memcpy(tvb.data, cmdList->VtxBuffer.Data, numVertices * sizeof(ImDrawVert));
        std::memcpy(tib.data, cmdList->IdxBuffer.Data, numIndices  * sizeof(ImDrawIdx));

        u32 idxOffset = 0;
        for (int cmdi = 0; cmdi < cmdList->CmdBuffer.Size; ++cmdi) {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[cmdi];

            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmdList, pcmd);
            } else {
                const u16 xx = static_cast<u16>(std::max(pcmd->ClipRect.x, 0.0f));
                const u16 yy = static_cast<u16>(std::max(pcmd->ClipRect.y, 0.0f));
                bgfx::setScissor(xx, yy,
                    static_cast<u16>(std::min(pcmd->ClipRect.z, 65535.0f) - xx),
                    static_cast<u16>(std::min(pcmd->ClipRect.w, 65535.0f) - yy));

                bgfx::TextureHandle texHandle = m_fontTexture;
                ImTextureID texId = pcmd->GetTexID();
                if (texId) {
                    texHandle = *reinterpret_cast<bgfx::TextureHandle*>(texId);
                }
                bgfx::setTexture(0, m_u_texture, texHandle);

                bgfx::setState(
                    BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                    BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                );

                // Must re-bind VB/IB before every submit — bgfx consumes state on submit
                bgfx::setVertexBuffer(0, &tvb);
                bgfx::setIndexBuffer(&tib, idxOffset, pcmd->ElemCount);
                bgfx::submit(255, m_program);
            }

            idxOffset += pcmd->ElemCount;
        }
    }
}

void ImGuiRenderer::setupKeyMappings() {
}

void ImGuiRenderer::setupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.ChildRounding = 2.0f;
    style.PopupRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.95f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.40f, 0.60f, 0.80f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.50f, 0.70f, 0.90f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.60f, 0.80f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.40f, 0.60f, 0.80f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.50f, 0.70f, 0.90f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.60f, 0.80f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 0.90f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.25f, 0.95f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.30f, 0.50f, 0.80f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.45f, 0.65f, 0.90f);
    colors[ImGuiCol_TabActive] = ImVec4(0.30f, 0.55f, 0.75f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.20f, 0.35f, 0.90f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.30f, 0.50f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.40f, 0.70f, 0.90f, 0.80f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.80f, 1.00f, 1.00f);
}

}
