#pragma once

#include <Engine/Core/Widget.h>

namespace WEngine
{
    class EngineControlWidget : public Widget
    {
    public:
        using Widget::Widget;

    private:

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;
    };

}

