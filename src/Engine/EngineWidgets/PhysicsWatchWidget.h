#pragma once

#include <Engine/Core/Widget.h>

namespace WEngine
{
    class SimulatableObject;
    class PhysicsWatchWidget : public Widget
    {
    public:
        using Widget::Widget;

    private:

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;

    private:
        void PrintCircleArea(const SimulatableObject* obj);
        void PrintRectArea(const SimulatableObject* obj);
    };

}

