#pragma once

#include <Engine/Core/Widget.h>


namespace WEngine
{
    class GameSystemWidget : public Widget
    {
    public:
        using Widget::Widget;

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;
    };
}
