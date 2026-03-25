#pragma once

#include "core/types.hpp"
#include "utils/spline.hpp"
#include "game/entities/track.hpp"
#include <memory>

namespace viber {

enum class EditorTool {
    Select = 0,
    Road = 1,
    Terrain = 2,
    Props = 3,
};

class TrackEditor {
public:
    TrackEditor();
    ~TrackEditor();
    
    void init();
    void shutdown();
    
    void update(float deltaTime);
    void render(class Renderer& renderer, const class Camera& camera);
    
    void setCurrentTool(EditorTool tool) { m_currentTool = tool; }
    EditorTool getCurrentTool() const { return m_currentTool; }
    
    void setRoadWidth(float width);
    float getRoadWidth() const { return m_roadWidth; }
    
    void setBrushSize(float size) { m_brushSize = size; }
    float getBrushSize() const { return m_brushSize; }
    
    void setBrushStrength(float strength) { m_brushStrength = strength; }
    float getBrushStrength() const { return m_brushStrength; }
    
    void toggleGrid() { m_showGrid = !m_showGrid; }
    bool isGridVisible() const { return m_showGrid; }
    
    void newTrack();
    bool saveTrack(const std::string& path);
    bool loadTrack(const std::string& path);
    
    void addControlPoint(const vec3& position);
    void clearRoad();
    
private:
    void handleRoadTool(const vec3& worldPos);
    void handleTerrainTool(const vec3& worldPos);
    void renderGrid(Renderer& renderer, const Camera& camera);
    void renderControlPoints();
    
    std::unique_ptr<Track> m_track;
    EditorTool m_currentTool = EditorTool::Road;
    
    float m_roadWidth = 12.0f;
    float m_brushSize = 10.0f;
    float m_brushStrength = 1.0f;
    
    bool m_showGrid = true;
    float m_gridSize = 100.0f;
    int m_gridDivisions = 20;
    
    int m_selectedPointIndex = -1;
};

}
