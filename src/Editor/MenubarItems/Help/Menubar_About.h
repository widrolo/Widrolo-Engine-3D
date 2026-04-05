#pragma once

#include <Editor/Types/MenubarItem.h>

namespace WEditor
{
    class Menubar_About : public MenubarItem
    {
    public:
        using MenubarItem::MenubarItem;

    protected:
        void Setup() override;
        void OnClick() override;

    };
}
