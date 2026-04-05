#pragma once
#include <string>
#include <Engine/imgui/imgui.h>

namespace WEditor
{
    class MenubarHandler;
    class MenubarItem
    {
        friend MenubarHandler;
    public:
        MenubarItem() : m_entryName("No Name") {}
        virtual ~MenubarItem() = default;

    protected:
        std::string m_entryName;

    public:
        virtual void Setup() = 0;
    protected:
        virtual void OnClick() = 0;
    };
}
