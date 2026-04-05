#include "WidgetHandler.h"

#include <Engine/EngineWidgets/SystemWidget.h>
#include <Engine/EngineWidgets/EngineControlWidget.h>
#include <Engine/EngineWidgets/StatisticsWidgets.h>
#include <Engine/EngineWidgets/TimingsWidget.h>
#include <Engine/EngineWidgets/PeripheralWidget.h>
#include <Engine/EngineWidgets/GameSystemWidget.h>
#include <Engine/EngineWidgets/SectorWatchWidget.h>
#include <Engine/EngineWidgets/PhysicsWatchWidget.h>
#include <Engine/EngineWidgets/DebugFlagsWidget.h>

#include <Engine/Types/CoreSystems.h>

#include "InputHandler.h"
#include <Engine/Core/System/Memory.h>

#include "Engine/Util/Log.h"
using namespace WEngine;

WidgetHandler::WidgetHandler()
{
    InitSystemWidgets();
}

WidgetHandler::~WidgetHandler()
{

}

void WidgetHandler::InitSystemWidgets()
{
    m_systemWidgets[(uint16)SysWidgetTypes::System]         = WAllocator::Construct<SystemWidget>();
    m_systemWidgets[(uint16)SysWidgetTypes::EngineControl]  = WAllocator::Construct<EngineControlWidget>();
    m_systemWidgets[(uint16)SysWidgetTypes::GameSystem]     = WAllocator::Construct<GameSystemWidget>();
    m_systemWidgets[(uint16)SysWidgetTypes::Statistics]     = WAllocator::Construct<StatisticsWidgets>();
    m_systemWidgets[(uint16)SysWidgetTypes::Timings]        = WAllocator::Construct<TimingsWidget>();
    m_systemWidgets[(uint16)SysWidgetTypes::Peripherals]    = WAllocator::Construct<PeripheralWidget>();
    m_systemWidgets[(uint16)SysWidgetTypes::SectorWatch]    = WAllocator::Construct<SectorWatchWidget>();
    m_systemWidgets[(uint16)SysWidgetTypes::PhysicsWatch]   = WAllocator::Construct<PhysicsWatchWidget>();
    m_systemWidgets[(uint16)SysWidgetTypes::DebugFlags]     = WAllocator::Construct<DebugFlagsWidget>();


    for (uint16 i = 0; i < (uint16)SysWidgetTypes::SysWidget_Count; i++)
        m_systemWidgets[i]->Setup();

    m_systemWidgets[(uint16)SysWidgetTypes::System]->m_open = true;
    m_systemWidgets[(uint16)SysWidgetTypes::EngineControl]->m_open = true;
    m_systemWidgets[(uint16)SysWidgetTypes::GameSystem]->m_open = true;
    m_systemWidgets[(uint16)SysWidgetTypes::Statistics]->m_open = true;
}

void WidgetHandler::DrawWidgets()
{

    const auto input = CoreSystems::GetInputHandler();

    const auto& stat = m_systemWidgets[(uint16)SysWidgetTypes::Statistics];
    const auto& control = m_systemWidgets[(uint16)SysWidgetTypes::EngineControl];
    const auto& sys = m_systemWidgets[(uint16)SysWidgetTypes::System];
    const auto& gameSys = m_systemWidgets[(uint16)SysWidgetTypes::GameSystem];

    if (input->GetKeyPressed(WKey::DEBUG1))
        sys->m_open = !sys->m_open;

    if (input->GetKeyPressed(WKey::DEBUG2))
        gameSys->m_open = !gameSys->m_open;

    if (input->GetKeyPressed(WKey::DEBUG12))
    {
        m_widgetsEnabled = !m_widgetsEnabled;
        if (m_widgetsEnabled)
        {
            stat->m_open = true;
            control->m_open = true;
            sys->m_open = true;
            gameSys->m_open = true;
        }
        else
        {
            for (uint16 i = 0; i < (uint16)SysWidgetTypes::SysWidget_Count; i++)
                m_systemWidgets[i]->m_open = false;
            for (const auto& widget : m_gameWidgets)
            {
                if (auto lw = widget.lock())
                    lw->m_open = false;

            }
        }
    }

    for (uint16 i = 0; i < (uint16)SysWidgetTypes::SysWidget_Count; i++)
        if (m_systemWidgets[i]->m_open)
            m_systemWidgets[i]->RenderWidget();

    TimingsWidget* time = (TimingsWidget*)m_systemWidgets[(uint16)SysWidgetTypes::Timings];

    time->CountTime();

    // im trying everything vro, this shit will soon be nasa grade code.
    auto widgetSnapshot = m_gameWidgets;
    for (auto& weak : widgetSnapshot)
    {
        if (auto widget = weak.lock())
        {
            widget->RenderWidget();
        }
    }
}

void WidgetHandler::AddGameWidget(const std::shared_ptr<Widget>& widget)
{
    widget->Setup();
    m_gameWidgets.push_back(widget);
}

void WidgetHandler::RemoveGameWidget(const std::shared_ptr<Widget>& widget)
{
    m_gameWidgets.erase(
        std::remove_if(
            m_gameWidgets.begin(),
            m_gameWidgets.end(),
            [&widget](const std::weak_ptr<Widget>& weak)
            {
                auto locked = weak.lock();
                return !locked || locked == widget;
            }),
        m_gameWidgets.end());
}
