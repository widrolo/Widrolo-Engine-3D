#pragma once

#include <Engine/Core/Widget.h>

namespace WEditor
{
    class ComponentList : public WEngine::Widget
    {
    public:
        using Widget::Widget;

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;
    private:
        const char* m_entityName;
        float m_entityPos[2] = { 0.0f, 0.0f };
        float m_entitySize[2] = { 0.0f, 0.0f };
    private:
        void ShowComponentsInEntity();
    };
}

