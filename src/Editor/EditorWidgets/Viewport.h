#pragma once

#include <Engine/Core/Widget.h>

#include "Engine/Core/World/Entity.h"

namespace WEngine {
    class CameraComponent;
}

namespace WEditor
{
    class Viewport : public WEngine::Widget
    {
    public:
        using Widget::Widget;

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;

    private:
        void CheckForSizeChange(ImVec2 newRes);
    private:
        WEngine::Entity* m_viewportEntity;
        WEngine::CameraComponent* viewportCam;
        bool m_resDecided = false;
    };
}

