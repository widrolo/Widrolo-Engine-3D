#pragma once
#include "Engine/Core/Widget.h"

namespace WEngine
{
    class TimeWatchWidget : public Widget
    {
    public:
        using Widget::Widget;

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;
    };
}

