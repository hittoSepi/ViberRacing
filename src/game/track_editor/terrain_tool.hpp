#pragma once

#include "core/types.hpp"
#include "renderer/renderer.hpp"

namespace viber {
    class TrackEditor;
    
    class TerrainTool {
    public:
        enum class BrushMode {
            Raise,
            Lower,
            Smooth,
            Flatten
        };
        
        explicit TerrainTool(TrackEditor* editor);
        
        void onActivate();
        void onDeactivate();
        
        void update(const vec3& cursorPos, bool isMouseDown, bool isRightClick);
        void render(Renderer& renderer);
        
        void setBrushMode(BrushMode mode);
        BrushMode getBrushMode() const { return m_mode; }
        
        void setBrushRadius(float radius) { m_brushRadius = radius; }
        void setBrushStrength(float strength) { m_brushStrength = strength; }
        float getBrushRadius() const { return m_brushRadius; }
        float getBrushStrength() const { return m_brushStrength; }
        
    private:
        void applyBrush(const vec3& position, BrushMode mode);
        
        TrackEditor* m_editor = nullptr;
        vec3 m_brushPosition{0.0f};
        BrushMode m_mode = BrushMode::Raise;
        float m_brushRadius = 10.0f;
        float m_brushStrength = 1.0f;
    };
}
