#pragma once
#include <Engine/Core/Widget.h>
#include <sstream>
#include <iomanip>


namespace WEngine
{
	class StatisticsWidgets : public Widget
	{
	public:
		using Widget::Widget;

	public:
		void Setup() override;
	protected:
		void RenderInternal() override;
	private:
		void TopRow();
		void BottomRow();
		std::string UptimeToString(uint64 uptime);
	};

}


