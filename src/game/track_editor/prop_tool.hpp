#pragma once

#include "core/types.hpp"
#include "renderer/renderer.hpp"
#include <string>
#include <vector>

namespace viber {
    class TrackEditor;
    
    struct PropTemplate {
        std::string name;
        std::string modelPath;
        vec3 boundingBox;
        bool randomizeRotation;
    };
    
    class PropTool {
    public:
        explicit PropTool(TrackEditor* editor);
        
        void onActivate();
        void onDeactivate();
        
        void update(const vec3& cursorPos, bool isMouseDown, bool isRightClick);
        void render(Renderer& renderer);
        
        void setSelectedProp(int index);
        void setRotation(float degrees);
        void deleteSelected();
        void placeProp(const vec3& position);
        
        float getPropScale() const { return m_propScale; }
        void setPropScale(float scale) { m_propScale = scale; }
        
    private:
        TrackEditor* m_editor = nullptr;
        float m_propScale = 1.0f;
        std::vector<PropTemplate> m_availableProps;
        int m_selectedPropIndex = -1;
        float m_rotation = 0.0f;
        vec3 m_cursorPosition;
        bool m_wasMouseDown = false;
    };
}
