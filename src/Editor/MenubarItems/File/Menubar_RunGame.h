#pragma once

#include <Editor/Types/MenubarItem.h>

namespace WEditor
{
    class Menubar_RunGame : public MenubarItem
    {
    public:
        using MenubarItem::MenubarItem;

    protected:
        void Setup() override;
        void OnClick() override;

    };
}

