#pragma once

#include "core/types.hpp"

namespace viber {
    class TrackEditor;
    class Renderer;
}

class TerrainTool {
public:
    enum class BrushMode {
        Raise,
        Lower,
        Smooth,
        Flatten
    };
    
    explicit TerrainTool(viber::TrackEditor* editor);
    
    void onActivate();
    void onDeactivate();
    
    void update(const viber::vec3& cursorPos, bool isMouseDown, bool isRightClick);
    void render(viber::Renderer& renderer);
    
    void setBrushMode(BrushMode mode);
    BrushMode getBrushMode() const { return m_mode; }
    
private:
    void applyBrush(const viber::vec3& position, BrushMode mode);
    
    viber::TrackEditor* m_editor = nullptr;
    viber::vec3 m_brushPosition{0.0f};
    BrushMode m_mode = BrushMode::Raise;
};
