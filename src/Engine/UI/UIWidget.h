#pragma once

#include <Engine/Types/CommonTypes.h>
#include <Engine/Math/Vector.h>
#include <Engine/WTL/vector.h>
#include <string>

#include <Engine/UI/UIElement.h>

namespace WEngine
{
    class UIWidget
    {
    public:
        UIWidget();
        explicit UIWidget(UIElementType type);
        explicit UIWidget(UIElementType type, Vector2 position, Vector2 size);
        ~UIWidget();

    protected:
        uint32 m_ID;
        Vector2 m_position;
        Vector2 m_size;
        UIWidget* m_parent;
        wtl::vector<UIWidget*> m_children;
        UIElementType m_type;
        wtl::vector<std::string> m_styleClasses;

    public:
        void SetSize(const Vector2& size) { m_size = size; }
        void SetPosition(const Vector2& position) { m_position = position; }
        void SetParent(UIWidget* parent) { m_parent = parent; }
        void SetType(const UIElementType type) { m_type = type; }

        [[nodiscard]] Vector2 GetSize() const { return m_size; }
        [[nodiscard]] Vector2 GetPosition() const { return m_position; }
        [[nodiscard]] UIWidget* GetParent() const { return m_parent; }
        [[nodiscard]] UIElementType GetType() const { return m_type; }

        void AddChild(UIWidget* child);
        void RemoveChild(int32 childID);

    protected:
        static uint32 GetNextID();
        void DestructWidget();

    private:
        void DestructChildren();
        void PromoteChildren();
    };
}