#include "StatisticsWidgets.h"
#include <Engine/Core/Engine.h>
#include <Engine/Core/System/Memory.h>

#include "Engine/Core/System/GPU.h"


using namespace WEngine;

void StatisticsWidgets::Setup()
{
	m_widgetName = "Statistics";
	m_windowFlags |= ImGuiWindowFlags_NoTitleBar;
	m_windowFlags |= ImGuiWindowFlags_NoScrollbar;
	m_windowFlags |= ImGuiWindowFlags_NoMove;
	m_windowFlags |= ImGuiWindowFlags_NoResize;
	m_windowFlags |= ImGuiWindowFlags_NoCollapse;
	m_windowFlags |= ImGuiWindowFlags_NoNav;
}

void StatisticsWidgets::RenderInternal()
{
	SetPosition({0, 0});
	SetSize({1920, 50});
	//SetSize({1920, 150});

	if (ImGui::BeginTable("statisticsTable", 7))
	{
		ImGui::TableNextRow();
		TopRow();
		ImGui::TableNextRow();
		BottomRow();
		ImGui::EndTable();
	}
}

void StatisticsWidgets::TopRow()
{
	ImGui::TableNextColumn();
	ImGui::Text("Frame Rate: %.2ffps", 1 / Engine::GetDeltaTime());
	ImGui::TableNextColumn();
	ImGui::Text("Ram Usage: %.2fKB", (float64)WAllocator::GetMemoryUsage() / KB);
	ImGui::TableNextColumn();
	ImGui::Text("Uptime: %s", UptimeToString(Engine::GetUptime()).c_str());
}
void StatisticsWidgets::BottomRow()
{
	ImGui::TableNextColumn();
	ImGui::Text("Frame Time: %.3fms", Engine::GetDeltaTime());
	ImGui::TableNextColumn();
	ImGui::Text("Vram Usage: %.2fMB", (float64)GPU::GetVramUsage() / MB);
	ImGui::TableNextColumn();
	ImGui::Text("Draw calls: %u", GPU::GetDrawCallCountLastFrame());
}

std::string StatisticsWidgets::UptimeToString(uint64 uptime)
{
	uint64_t days = uptime / 86400;
	uptime %= 86400;

	uint64_t hours = uptime / 3600;
	uptime %= 3600;

	uint64_t minutes = uptime / 60;
	uint64_t seconds = uptime % 60;

	std::string result;
	bool first = true;

	auto append = [&](uint64_t value, const char* suffix) {
		if (value > 0) {
			if (!first) result += ", ";
			result += std::to_string(value) + suffix;
			first = false;
		}
	};

	append(days, "d");
	append(hours, "h");
	append(minutes, "m");
	append(seconds, "s");

	// If input is 0 seconds, return "0s"
	return result.empty() ? "0s" : result;
}
