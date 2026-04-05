#pragma once

#include <Engine/WTL/vector.h>
#include <Engine/UI/UIWidget.h>

namespace WEngine
{
    class UIHandler
    {
    public:
        UIHandler();
        ~UIHandler();

    private:
        wtl::vector<UIWidget*> m_widgets;

    public:
        void AddUIWidget(UIWidget* widget);
        void RemoveUIWidget(UIWidget* widget);
        void RemoveAllUIWidgets();

        void DrawWidgets();
    };
}

