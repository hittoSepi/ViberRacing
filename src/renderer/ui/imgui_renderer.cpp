#include "imgui_renderer.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <bgfx/bgfx.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace viber {

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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    setupStyle();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    
    m_layout.begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
    
    m_u_texture = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);
    
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    
    m_fontTexture = bgfx::createTexture2D(
        static_cast<u16>(width), static_cast<u16>(height),
        false, 1, bgfx::TextureFormat::BGRA8,
        0, bgfx::copy(pixels, width * height * 4)
    );
    
    io.Fonts->TexID = reinterpret_cast<ImTextureID>(m_fontTexture.idx);
    
    m_initialized = true;
    spdlog::info("ImGui initialized");
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
        bx::mtxOrtho(ortho, 0.0f, width, height, 0.0f, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
        bgfx::setViewTransform(255, nullptr, ortho);
        bgfx::setViewRect(255, 0, 0, static_cast<u16>(width), static_cast<u16>(height));
    }
    
    for (int n = 0; n < drawData->CmdListsCount; ++n) {
        const ImDrawList* cmdList = drawData->CmdLists[n];
        
        const ImDrawVert* vtxBuf = cmdList->VtxBuffer.Data;
        const ImDrawIdx* idxBuf = cmdList->IdxBuffer.Data;
        
        if (!bgfx::isValid(m_vbh) || bgfx::getAvailTransientVertexBuffer(
            static_cast<u32>(cmdList->VtxBuffer.Size), m_layout) < cmdList->VtxBuffer.Size) {
            bgfx::TransientVertexBuffer tvb;
            bgfx::allocTransientVertexBuffer(&tvb, 
                static_cast<u32>(cmdList->VtxBuffer.Size), m_layout);
            std::memcpy(tvb.data, vtxBuf, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
            bgfx::setVertexBuffer(0, &tvb);
        }
        
        if (!bgfx::isValid(m_ibh) || bgfx::getAvailTransientIndexBuffer(
            static_cast<u32>(cmdList->IdxBuffer.Size)) < cmdList->IdxBuffer.Size) {
            bgfx::TransientIndexBuffer tib;
            bgfx::allocTransientIndexBuffer(&tib, 
                static_cast<u32>(cmdList->IdxBuffer.Size), 
                sizeof(ImDrawIdx) == 4);
            std::memcpy(tib.data, idxBuf, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
            bgfx::setIndexBuffer(&tib);
        }
        
        u32 idxOffset = 0;
        for (int cmdi = 0; cmdi < cmdList->CmdBuffer.Size; ++cmdi) {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[cmdi];
            
            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmdList, pcmd);
            } else {
                const u16 xx = static_cast<u16>(bx::max(pcmd->ClipRect.x, 0.0f));
                const u16 yy = static_cast<u16>(bx::max(pcmd->ClipRect.y, 0.0f));
                bgfx::setScissor(xx, yy,
                    static_cast<u16>(bx::min(pcmd->ClipRect.z, 65535.0f) - xx),
                    static_cast<u16>(bx::min(pcmd->ClipRect.w, 65535.0f) - yy));
                
                bgfx::setTexture(0, m_u_texture, {pcmd->TextureId ? 
                    static_cast<u16>(reinterpret_cast<uintptr_t>(pcmd->TextureId)) : 
                    m_fontTexture.idx});
                
                bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA);
                
                bgfx::submit(255, m_program, 0, 
                    bgfx::Discard::None, idxOffset, pcmd->ElemCount);
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
