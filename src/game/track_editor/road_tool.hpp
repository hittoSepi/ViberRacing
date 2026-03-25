#pragma once

#include "core/types.hpp"
#include "renderer/renderer.hpp"

namespace viber {
    class TrackEditor;
    
    class RoadTool {
    public:
        explicit RoadTool(TrackEditor* editor);
        
        void onActivate();
        void onDeactivate();
        
        void update(const vec3& cursorPos, bool isMouseDown, bool isRightClick);
        void render(Renderer& renderer);
        
        void setRoadWidth(float width);
        float getRoadWidth() const { return m_roadWidth; }
        
    private:
        TrackEditor* m_editor = nullptr;
        float m_roadWidth = 10.0f;
        std::vector<vec3> m_controlPoints;
        bool m_wasMouseDown = false;
    };
}
