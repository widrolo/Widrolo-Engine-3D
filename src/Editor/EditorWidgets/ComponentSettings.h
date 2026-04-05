#pragma once

#include <Engine/Core/Widget.h>

#include "Editor/Types/ComponentSettingDefinition.h"

namespace WEditor
{
    class ComponentSettings : public WEngine::Widget
    {
    public:
        using Widget::Widget;

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;
    private:
        void ShowComponent();
        void ShowOption(const ComponentOption& option, uint8 optionNumber);
    };
}

