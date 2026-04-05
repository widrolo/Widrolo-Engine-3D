#pragma once

#include <Engine/Types/CommonTypes.h>
#include <Engine/WTL/vector.h>
#include <array>

namespace WEngine
{
    class Widget;
}

namespace WEditor
{
    class EditorUIHandler
    {
    public:
        EditorUIHandler();
        ~EditorUIHandler();
    private:
        wtl::vector<WEngine::Widget*> m_uiWidgets;

        void InitSystemWidgets();
    public:
        void DrawWidgets();
        void AddEditorWidget(WEngine::Widget* widget, bool openOnAdd);
        void RemoveEditorWidget(const WEngine::Widget* widget);

    };
}

