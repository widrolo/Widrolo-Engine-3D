#include "TimingsWidget.h"
#include <Engine/Core/Engine.h>
#include <Engine/imgui/implot.h>
#include <sstream>
#include <algorithm>
#include <cmath>

#include "../Core/System/Memory.h"

using namespace WEngine;

void TimingsWidget::Setup()
{
	m_widgetName = "Engine Timings";
	m_windowFlags |= ImGuiWindowFlags_NoResize;

	m_beginTimesBuf = (float32*)wNew(MaxTimings * sizeof(float32));
	m_sectorTimesBuf = (float32*)wNew(MaxTimings * sizeof(float32));
	m_tickTimesBuf = (float32*)wNew(MaxTimings * sizeof(float32));
	m_physicsTimesBuf = (float32*)wNew(MaxTimings * sizeof(float32));
	m_widgetTimesBuf = (float32*)wNew(MaxTimings * sizeof(float32));
	m_drawTimesBuf = (float32*)wNew(MaxTimings * sizeof(float32));

}

void TimingsWidget::CountTime()
{
	AddTime(m_beginTimes, Engine::GetFrameBegin());
	AddTime(m_sectorTimes, Engine::GetSectorLoad());
	AddTime(m_tickTimes, Engine::GetEntityTick());
	AddTime(m_physicsTimes, Engine::GetPhysicsTick());
	AddTime(m_widgetTimes, Engine::GetWidgetDraw());
	AddTime(m_drawTimes, Engine::GetDraw());
}

void TimingsWidget::RenderInternal()
{
	SetSize({ 400, 500 });

	std::stringstream begin, widget, sector, entity, physics, draw;

	begin << "Begin: " << Engine::GetFrameBegin() << "uS";
	sector << "Sector: " << Engine::GetSectorLoad() << "uS";
	entity << "Entity: " << Engine::GetEntityTick() << "uS";
	physics << "Physics: " << Engine::GetPhysicsTick() << "uS";
	widget << "Widgets: " << Engine::GetWidgetDraw() << "uS";
	draw << "Draw: " << Engine::GetDraw() << "uS";

	MakePlot(m_beginTimes, begin.str().c_str(), 300, m_beginTimesBuf);
	MakePlot(m_sectorTimes, sector.str().c_str(), 2000, m_sectorTimesBuf);
	MakePlot(m_tickTimes, entity.str().c_str(), 1000, m_tickTimesBuf);
	MakePlot(m_physicsTimes, physics.str().c_str(), 300, m_physicsTimesBuf);
	MakePlot(m_widgetTimes, widget.str().c_str(), 2000, m_widgetTimesBuf);
	MakePlot(m_drawTimes, draw.str().c_str(), 2500, m_drawTimesBuf);
}

void TimingsWidget::AddTime(wtl::deque<float32>& buf, float32 time)
{
	if (buf.size() >= MaxTimings)
		buf.pop_front();

	buf.push_back(time);
}



void TimingsWidget::MakePlot(wtl::deque<float32>& timings, const char* name, uint32 maxY, float32* buf)
{
	DequeToCArray(timings, MaxTimings, buf);
	ImPlotFlags flag = ImPlotFlags_NoInputs;
	if (ImPlot::BeginPlot(name, ImVec2(-1, 0), flag))
	{
		ImPlot::SetupAxes("hist", "uS");
		ImPlot::SetupAxesLimits(0, MaxTimings, 0, maxY);
		ImPlot::PlotLine(name, buf, MaxTimings);
		ImPlot::EndPlot();
	}
}
