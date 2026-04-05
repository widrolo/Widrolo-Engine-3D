#pragma once

#include <tinyxml2/tinyxml2.h>
#include "UIWidget.h"
#include <array>
#include <functional>

namespace WEngine
{
    enum class ParsableUIElementType : uint8
    {
        Invalid = 0,
        Container,
        TextLabel,
        Image,
        Button,
        Slider,
        Checkbox,
        Seperator,
        TextInput,
        Dropdown,
        ParsableCount
    };

    class UIParser
    {
    public:
        static UIWidget* ParseUISheet(const tinyxml2::XMLDocument& document);

    private:
        using UIFactory = std::function<UIWidget*()>;
        static const std::array<std::pair<std::string, UIFactory>,
        (uint8)ParsableUIElementType::ParsableCount> Parsables;

        static void RecourseParseWidget(const tinyxml2::XMLNode* node, UIWidget* parent);

        static void ParseElement_Container(const tinyxml2::XMLNode* node, UIWidget* element);
        static void ParseElement_TextLabel(const tinyxml2::XMLNode* node, UIWidget* element);
        static void ParseElement_Image(const tinyxml2::XMLNode* node, UIWidget* element);
        static void ParseElement_Button(const tinyxml2::XMLNode* node, UIWidget* element);
        static void ParseElement_Slider(const tinyxml2::XMLNode* node, UIWidget* element);
        static void ParseElement_Checkbox(const tinyxml2::XMLNode* node, UIWidget* element);
        static void ParseElement_Seperator(const tinyxml2::XMLNode* node, UIWidget* element);
        static void ParseElement_TextInput(const tinyxml2::XMLNode* node, UIWidget* element);
        static void ParseElement_Dropdown(const tinyxml2::XMLNode* node, UIWidget* element);
    };
}

