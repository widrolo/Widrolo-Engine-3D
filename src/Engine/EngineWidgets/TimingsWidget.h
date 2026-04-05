#pragma once

#include <Engine/Core/Widget.h>
#include <Engine/WTL/deque.h>
#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
	class TimingsWidget : public Widget
	{
	public:
		using Widget::Widget;

	private:
		wtl::deque<float32> m_beginTimes;
		wtl::deque<float32> m_sectorTimes;
		wtl::deque<float32> m_tickTimes;
		wtl::deque<float32> m_physicsTimes;
		wtl::deque<float32> m_widgetTimes;
		wtl::deque<float32> m_drawTimes;
		float32* m_beginTimesBuf = nullptr;
		float32* m_sectorTimesBuf = nullptr;
		float32* m_tickTimesBuf = nullptr;
		float32* m_physicsTimesBuf = nullptr;
		float32* m_widgetTimesBuf = nullptr;
		float32* m_drawTimesBuf = nullptr;

		_GLOBAL_CEX_ int16 MaxTimings = 500;

	public:
		void Setup() override;
		void CountTime();
	protected:
		void RenderInternal() override;
	private:
		void AddTime(wtl::deque<float32>& buf, float32 time);
		void MakePlot(wtl::deque<float32> &timings, const char* name, uint32 maxY, float32* buf);
	};
}

