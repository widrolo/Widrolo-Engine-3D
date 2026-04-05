#include "Widget.h"

#include <iostream>

#include "Engine/Types/CoreSystems.h"
#include "Engine/Util/Log.h"
#include "Handlers/RenderHandler.h"

using namespace WEngine;

Widget::Widget()
{
	m_widgetName = "No Name Widget";
	m_windowFlags = 0;
	m_open = false;

	m_nodeContext = ImNode::CreateEditor();
	Setup();
}

void Widget::RenderWidget()
{
	if (!m_open)
		return;
	ImGui::Begin(m_widgetName.c_str(), &m_open, m_windowFlags);

	RenderInternal();

	ImGui::End();
}

void Widget::Setup()
{


}

void Widget::RenderInternal()
{
}

void Widget::SetPosition(const Vector2 &position)
{
	Vector2 internal = CoreSystems::GetRenderHandler()->GetInternalResolution();
	Vector2 window = CoreSystems::GetRenderHandler()->GetWindowResolution();

	float32 yOffset = window.y - internal.y;

	ImGui::SetWindowPos({position.x, position.y + yOffset});

}
void Widget::SetSize(const Vector2 &size)
{
	ImGui::SetWindowSize({size.x, size.y}, 0);
}

void Widget::DequeToCArray(wtl::deque<float32> &buf, uint16 maxSize, float32 *outData)
{
	int16 count = (int16)std::min(buf.size(), static_cast<size_t>(maxSize));
	for (size_t i = 0; i < count; ++i)
		outData[i] = buf[i];

	// NaN the rest if deque isnt full
	for (size_t i = count; i < maxSize; ++i)
		outData[i] = NAN;
}
