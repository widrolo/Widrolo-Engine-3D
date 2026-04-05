#pragma once

#include <array>
#include <Engine/WTL/vector.h>
#include <Editor/Types/MenubarItem.h>

namespace WEditor
{
    enum class MenubarItemCategories : uint8
    {
        File = 0,
        Edit,
        View,
        Tools,
        Window,
        Help,
        Close,
        Categories_Count
    };

    extern std::string MenubarItemCategoryNames[];

    class MenubarHandler
    {
    public:
        MenubarHandler();
        ~MenubarHandler();

    private:
        std::array<wtl::vector<MenubarItem*>, (uint8)MenubarItemCategories::Categories_Count> m_Items;
        uint8 m_categoryCounter;

    public:
        void Render();

    private:
        void Init();
        template<class T = MenubarItem>
        void AddNewMenubarItem(MenubarItemCategories category)
        {
            m_Items[(uint8)category].push_back(WAllocator::Construct<T>());
            m_Items[(uint8)category][m_Items[(uint8)category].size() - 1]->Setup();
        }
    };
}
