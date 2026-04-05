#pragma once
#include "Engine/Core/Widget.h"


namespace WEditor
{
    class NodeTestWidget : public WEngine::Widget
    {
    public:
        using Widget::Widget;

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;
    };
}

