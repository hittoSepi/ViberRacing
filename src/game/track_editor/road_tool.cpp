#pragma once

#include "core/types.hpp"

namespace viber {
    class TrackEditor;
}

class RoadTool {
public:
    explicit RoadTool(viber::TrackEditor* editor);
    
    void onActivate();
    void onDeactivate();
    
    void update(const viber::vec3& cursorPos, bool isMouseDown, bool isRightClick);
    void render(class viber::Renderer& renderer);
    
    void setRoadWidth(float width);
    
private:
    viber::TrackEditor* m_editor = nullptr;
    bool m_wasMouseDown = false;
};
