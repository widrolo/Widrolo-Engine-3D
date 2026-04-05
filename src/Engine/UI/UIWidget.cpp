#include "UIWidget.h"

#include <Engine/EngineDefines.h>
#include <algorithm>

using namespace WEngine;

UIWidget::UIWidget() :
    m_ID(GetNextID()),
    m_position(Vector2::Zero),
    m_size(Vector2::One),
    m_parent(nullptr),
    m_type(UIElementType::None)
{

}
UIWidget::UIWidget(const UIElementType type) :
    m_ID(GetNextID()),
    m_position(Vector2::Zero),
    m_size(Vector2::One),
    m_parent(nullptr),
    m_type(type)
{

}
UIWidget::UIWidget(const UIElementType type, const Vector2 position, const Vector2 size) :
    m_ID(GetNextID()),
    m_position(position),
    m_size(size),
    m_parent(nullptr),
    m_type(type)
{

}
UIWidget::~UIWidget()
{
    DestructWidget();
}
void UIWidget::AddChild(UIWidget *child)
{
    m_children.push_back(child);
}
void UIWidget::RemoveChild(const int32 childID)
{
    m_children.erase(std::ranges::find_if(m_children,
        [&](const UIWidget *child){ return child->m_ID == childID; }));
}
uint32 UIWidget::GetNextID()
{
    static uint32 nextID = 0;
    return nextID++;
}

void UIWidget::DestructWidget()
{
    if (EngineSettings::uiPromoteChildrenOnDelete)
        PromoteChildren();
    else
        DestructChildren();
}
void UIWidget::DestructChildren()
{
    for (const auto& child : m_children)
        WAllocator::Destruct(child);
}
void UIWidget::PromoteChildren()
{
    for (const auto& child : m_children)
        child->m_parent = m_parent;
}
