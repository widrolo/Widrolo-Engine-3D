#pragma once

#include <Engine/imgui/imgui.h>

#include <string>

#include "Engine/Math/Vector.h"
#include <Engine/WTL/deque.h>

namespace WEditor
{
	class EditorUIHandler;
}

namespace WEngine
{
	class WidgetHandler;
	class SystemWidget;
	class GameSystemWidget;
	/**
	 * The base class for all widgets in the WEngine. A widget is a graphical user interface element that can be
	 * displayed and interacted with by the user.
	 */
	class Widget
	{
		friend WidgetHandler;
		friend WEditor::EditorUIHandler;
		friend SystemWidget; // crazy
		friend GameSystemWidget; // crazy no.2
	public:
		Widget();
		virtual ~Widget() = default;

	protected:
		std::string m_widgetName;			///< The name of the widget
		ImGuiWindowFlags m_windowFlags{};	///< The flags for the ImGui window
		bool m_open;						///< Whether the widget is currently open or not

	public:
		/**
		 * Renders the widget on the screen.
		 * @note This function should not be called directly, but rather through WidgetHandler::DrawWidgets().
		 */
		void RenderWidget();
		/**
	 	 * Sets up the widget for rendering. This function is called once when the widget is constructed and should be
	 	 * overridden by subclasses to perform any necessary setup.
		 */
		virtual void Setup();
		/**
		  * Forcefully opens or closes the widget
		 */
		void SetOpenState(const bool open) { m_open = open; }

	protected:
		/**
	 	 * Renders the internal contents of the widget. This function is called by RenderWidget() and should be overridden
	 	 * by subclasses to render their own contents.
	 	 */
		virtual void RenderInternal();
		/**
		 * Sets the position of the widget on the screen.
		 * @param position The new position of the widget as a Vector2 object.
		 */
		void SetPosition(const Vector2& position);
		/**
		 * Sets the size of the widget on the screen.
		 * @param size The new size of the widget as a Vector2 object.
		 */
		void SetSize(const Vector2& size);

		void DequeToCArray(wtl::deque<float32>& buf, uint16 maxSize , float32* outData);

	};
}



