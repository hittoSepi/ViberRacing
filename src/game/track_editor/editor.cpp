#include "editor.hpp"
#include "renderer/renderer.hpp"
#include "renderer/debug_draw.hpp"
#include <spdlog/spdlog.h>

namespace viber {

TrackEditor::TrackEditor() {
    m_track = std::make_unique<Track>();
}

TrackEditor::~TrackEditor() {
    shutdown();
}

void TrackEditor::init() {
    m_track = std::make_unique<Track>();
    spdlog::info("TrackEditor initialized");
}

void TrackEditor::shutdown() {
    m_track.reset();
}

void TrackEditor::update(float) {
}

void TrackEditor::render(Renderer& renderer, const Camera& camera) {
    if (m_showGrid) {
        renderGrid(renderer, camera);
    }
    
    renderControlPoints();
}

void TrackEditor::setRoadWidth(float width) {
    m_roadWidth = width;
    if (m_track) {
        m_track->setRoadWidth(width);
    }
}

void TrackEditor::newTrack() {
    m_track = std::make_unique<Track>();
    m_track->setRoadWidth(m_roadWidth);
    m_selectedPointIndex = -1;
    spdlog::info("Created new track");
}

bool TrackEditor::saveTrack(const std::string& path) {
    if (!m_track) return false;
    
    if (m_track->saveToFile(path)) {
        spdlog::info("Track saved to: {}", path);
        return true;
    }
    return false;
}

bool TrackEditor::loadTrack(const std::string& path) {
    auto loadedTrack = std::make_unique<Track>();
    
    if (loadedTrack->loadFromFile(path)) {
        m_track = std::move(loadedTrack);
        m_selectedPointIndex = -1;
        spdlog::info("Track loaded from: {}", path);
        return true;
    }
    
    return false;
}

void TrackEditor::addControlPoint(const vec3& position) {
    if (!m_track) return;
    
    auto& spline = m_track->getSpline();
    spline.addControlPoint(position);
    
    m_track->generateFromSpline(spline);
    
    spdlog::info("Added control point at ({:.1f}, {:.1f}, {:.1f})", 
        position.x, position.y, position.z);
}

void TrackEditor::clearRoad() {
    if (!m_track) return;
    
    m_track->getSpline().clear();
    m_track->generateFromSpline(m_track->getSpline());
    m_selectedPointIndex = -1;
    
    spdlog::info("Road cleared");
}

void TrackEditor::handleRoadTool(const vec3&) {
}

void TrackEditor::handleTerrainTool(const vec3&) {
}

void TrackEditor::renderGrid(Renderer&, const Camera& camera) {
    static DebugDraw debugDraw;
    static bool initialized = false;
    
    if (!initialized) {
        debugDraw.init();
        initialized = true;
    }
    
    debugDraw.beginFrame(camera.getViewMatrix(), camera.getProjectionMatrix());
    debugDraw.drawGrid(vec3(0.0f), vec3(0.0f, 1.0f, 0.0f), 
                        m_gridSize, m_gridDivisions, vec4(0.3f, 0.3f, 0.3f, 0.5f));
    debugDraw.endFrame();
}

void TrackEditor::renderControlPoints() {
    if (!m_track) return;
    
    const auto& points = m_track->getSpline().getControlPoints();
    
    for (size_t i = 0; i < points.size(); ++i) {
        const vec3& point = points[i];
        bool isSelected = static_cast<int>(i) == m_selectedPointIndex;
        vec4 color = isSelected ? vec4(1.0f, 1.0f, 0.0f, 1.0f) : vec4(0.0f, 1.0f, 0.0f, 1.0f);
        
        // Debug visualization would go here
    }
}

}
