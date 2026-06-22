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
        std::array<float32, 3> m_entityPos{};
        std::array<float32, 3> m_entityRot{};
        std::array<float32, 3> m_entitySize{};
    private:
        void ShowComponentsInEntity();
        void ComponentDropdown();
    };
}

