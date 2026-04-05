#include "MenubarHandler.h"

#include <Editor/MenubarItems/File/Menubar_RunGame.h>
#include <Editor/MenubarItems/File/Menubar_Exit.h>

#include <Editor/MenubarItems/Tools/Menubar_ResFind.h>
#include "Editor/MenubarItems/Tools/Menubar_AnimEditor.h"

#include <Editor/MenubarItems/Help/Menubar_About.h>
#include <Editor/MenubarItems/Help/Menubar_NodeTest.h>


namespace WEditor
{
    std::string MenubarItemCategoryNames[(int)MenubarItemCategories::Categories_Count] =
    {
        "File",
        "Edit",
        "View",
        "Tools",
        "Window",
        "Help",
        "Close"
    };
}

using namespace WEditor;

MenubarHandler::MenubarHandler()
{
    Init();
}

MenubarHandler::~MenubarHandler()
{

}

void MenubarHandler::Render()
{
    ImGui::BeginMainMenuBar();


    int canN = 0;
    for (const auto& category : m_Items)
    {
        if (ImGui::BeginMenu(MenubarItemCategoryNames[canN].c_str()))
        {
            for (const auto item : category)
            {
                if (ImGui::MenuItem(item->m_entryName.c_str()))
                    item->OnClick();
            }
            ImGui::EndMenu();
        }
        canN++;
    }

    ImGui::EndMainMenuBar();
}

void MenubarHandler::Init()
{
    AddNewMenubarItem<Menubar_RunGame>(MenubarItemCategories::File);
    AddNewMenubarItem<Menubar_Exit>(MenubarItemCategories::File);

    AddNewMenubarItem<Menubar_ResFind>(MenubarItemCategories::Tools);
    AddNewMenubarItem<Menubar_AnimEditor>(MenubarItemCategories::Tools);

    AddNewMenubarItem<Menubar_About>(MenubarItemCategories::Help);
    AddNewMenubarItem<Menubar_NodeTest>(MenubarItemCategories::Help);
}

