#pragma once

#include <Engine/Types/CommonTypes.h>
#include <Engine/WTL/vector.h>
#include <array>
#include <memory>

namespace WEngine
{
    enum class SysWidgetTypes : uint16
    {
        System,
        EngineControl,
        GameSystem,
        Statistics,
        Timings,
        Peripherals,
        SectorWatch,
        PhysicsWatch,
        DebugFlags,
        RenderWatch,

        SysWidget_Count
    };

    class Widget;
    class SystemWidget;
    class GameSystemWidget;

    /**
     * Handles the creation, drawing and removal of engine UI components (Widgets).
     */
    class WidgetHandler
    {
        friend SystemWidget; // cursed
        friend GameSystemWidget;
    public:
        WidgetHandler();
        ~WidgetHandler();
    private:
        std::array<Widget*, (uint16)SysWidgetTypes::SysWidget_Count> m_systemWidgets;
        wtl::vector<std::weak_ptr<Widget>> m_gameWidgets;
        bool m_widgetsEnabled = true;
    public:
        /**
         * Initializes system widgets with their respective functionalities.
         */
        void InitSystemWidgets();
        /**
         * Draws all enabled system and game widgets.
         */
        void DrawWidgets();
        /**
         * Adds a new game widget to be handled and rendered.
         * @param widget Pointer to the game widget to be added.
         */
        void AddGameWidget(const std::shared_ptr<Widget>& widget);
        /**
         * Removes a game widget from being handled and rendered.
         * @param widget Pointer to the game widget to be removed.
         */
        void RemoveGameWidget(const std::shared_ptr<Widget>& widget);
    };
}
