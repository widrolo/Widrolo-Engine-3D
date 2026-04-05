#include "UIParser.h"

#include "Elements/Containers/Container.h"
#include "Elements/Labels/TextLabel.h"
#include "Elements/Images/Image.h"
#include "Elements/Buttons/Button.h"
#include "Elements/Sliders/Slider.h"
#include "Elements/Toggles/Checkbox.h"
#include "Elements/Seperators/Seperator.h"
#include "Elements/InputFields/TextInput.h"
#include "Elements/Dropdowns/Dropdown.h"
#include "Engine/Util/Log.h"

using namespace WEngine;

// I dont think these even need to be in enum class order for it to work.
const std::array<std::pair<std::string, UIParser::UIFactory>,
(uint8)ParsableUIElementType::ParsableCount> UIParser::Parsables =
{
    std::make_pair("container", [](){ return WAllocator::Construct<UIContainer,  UIElementType>(UIElementType::Container ); }),
    std::make_pair("textLabel", [](){ return WAllocator::Construct<UITextLabel,  UIElementType>(UIElementType::Label     ); }),
    std::make_pair("image",     [](){ return WAllocator::Construct<UIImage,      UIElementType>(UIElementType::Image     ); }),
    std::make_pair("button",    [](){ return WAllocator::Construct<UIButton,     UIElementType>(UIElementType::Button    ); }),
    std::make_pair("slider",    [](){ return WAllocator::Construct<UISlider,     UIElementType>(UIElementType::Slider    ); }),
    std::make_pair("checkbox",  [](){ return WAllocator::Construct<UICheckbox,   UIElementType>(UIElementType::Toggle    ); }),
    std::make_pair("seperator", [](){ return WAllocator::Construct<UISeperator,  UIElementType>(UIElementType::Seperator ); }),
    std::make_pair("textInput", [](){ return WAllocator::Construct<UITextInput,  UIElementType>(UIElementType::InputField); }),
    std::make_pair("dropdown",  [](){ return WAllocator::Construct<UIDropdown,   UIElementType>(UIElementType::Dropdown  ); })
};

UIWidget* UIParser::ParseUISheet(const tinyxml2::XMLDocument& document)
{
    auto sheetNode = document.FirstChildElement("sheet");

    if (!sheetNode)
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Document was invalid as it doesnt contain a first \"sheet\" element!");
        return nullptr;
    }

    UIWidget* sheet = WAllocator::Construct<UIWidget, UIElementType>(UIElementType::Root);

    RecourseParseWidget(sheetNode, sheet);

    return sheet;
}

void UIParser::RecourseParseWidget(const tinyxml2::XMLNode* node, UIWidget* parent)
{
    uint32 childCount = node->ChildElementCount();
    const tinyxml2::XMLNode* currentChild = node->FirstChildElement();
    for (uint32 i = 0; i < childCount; ++i)
    {
        std::string elementName = currentChild->Value();
        UIWidget* newWidget = nullptr;
        auto it = std::find_if(Parsables.begin(), Parsables.end(),
            [&](const std::pair<std::string, UIFactory>& pair) {
                return pair.first == elementName;
            });


        if (it == Parsables.end())
        {
            WLog::SetConsoleWarning();
            WLog::ConsoleLog(std::format("Unknown UI Element \"{}\"", elementName));
            currentChild = currentChild->NextSiblingElement();
            continue;
        }

        newWidget = it->second();

        newWidget->SetParent(parent);
        parent->AddChild(newWidget);

        switch (newWidget->GetType())
        {
            case UIElementType::None:
                break;
            case UIElementType::Root:
                break;
            case UIElementType::Container:
                ParseElement_Container(currentChild, newWidget);
                break;
            case UIElementType::Label:
                ParseElement_TextLabel(currentChild, newWidget);
                break;
            case UIElementType::Image:
                ParseElement_Image(currentChild, newWidget);
                break;
            case UIElementType::InputField:
                ParseElement_TextInput(currentChild, newWidget);
                break;
            case UIElementType::Seperator:
                ParseElement_Seperator(currentChild, newWidget);
                break;
            case UIElementType::Button:
                ParseElement_Button(currentChild, newWidget);
                break;
            case UIElementType::Toggle:
                ParseElement_Checkbox(currentChild, newWidget);
                break;
            case UIElementType::Slider:
                ParseElement_Slider(currentChild, newWidget);
                break;
            case UIElementType::Dropdown:
                ParseElement_Dropdown(currentChild, newWidget);
                break;
            default:
                break;
        }

        RecourseParseWidget(currentChild, newWidget);
        currentChild = currentChild->NextSiblingElement();
    }
}

void UIParser::ParseElement_Container(const tinyxml2::XMLNode *node, UIWidget *element)
{
    auto container = (UIContainer*)element;
}

void UIParser::ParseElement_TextLabel(const tinyxml2::XMLNode *node, UIWidget *element)
{
    auto container = (UITextLabel*)element;
}

void UIParser::ParseElement_Image(const tinyxml2::XMLNode *node, UIWidget *element)
{
    auto container = (UIImage*)element;
}

void UIParser::ParseElement_Button(const tinyxml2::XMLNode *node, UIWidget *element)
{
    auto container = (UIButton*)element;
}

void UIParser::ParseElement_Slider(const tinyxml2::XMLNode *node, UIWidget *element)
{
    auto container = (UISlider*)element;
}

void UIParser::ParseElement_Checkbox(const tinyxml2::XMLNode *node, UIWidget *element)
{
    auto container = (UICheckbox*)element;
}

void UIParser::ParseElement_Seperator(const tinyxml2::XMLNode *node, UIWidget *element)
{
    auto container = (UISeperator*)element;
}

void UIParser::ParseElement_TextInput(const tinyxml2::XMLNode *node, UIWidget *element)
{
    auto container = (UITextInput*)element;
}

void UIParser::ParseElement_Dropdown(const tinyxml2::XMLNode *node, UIWidget *element)
{
    auto container = (UIDropdown*)element;
}

