#pragma once

#include <Engine/Core/Widget.h>

namespace WEditor
{
    class EntityList : public WEngine::Widget
    {
    public:
        using Widget::Widget;

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;
    private:
        void ShowEntitiesInSector();
    };
}

