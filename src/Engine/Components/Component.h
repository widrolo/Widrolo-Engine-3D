#pragma once
#include <string>
#include <Engine/Types/CommonTypes.h>

#include "Engine/Math/Vector.h"

namespace WEngine
{
	struct ComponentArgs;
	class InputComponent; // This is sooo silly...
	class Entity;
	class Component
	{
	public:
		Component() = default;
		Component(Entity* e);
		virtual ~Component() = default;

	protected:
		std::string m_nameOfComponent = "{unkn}";
		std::string m_ownerName = "{unkn}";
		Entity* entity = nullptr;

		InputComponent* input;

	public:
		virtual void Awake(ComponentArgs ca);
		virtual void Start() {}
		virtual void Stop() {}
		virtual void Tick(float32 dt) {}
		virtual void PhysicsTick(float32 tr) {}
		virtual void Draw() {}
		virtual void DrawDebug() {}

		virtual void UnloadComponent() {}

		[[nodiscard]] virtual uint64 GetHash() const = 0;
		[[nodiscard]] std::string GetName() const;

		[[nodiscard]] Vector2 GetPosition() const;
	protected:
		[[deprecated]]
		void MakeErrorMsg(int line, std::string reason);
	};
}

#define COMP_HASH(h) public: uint64 GetHash() const override { return StaticGetHash(); } static constexpr uint64 StaticGetHash() { return h; }
#define COMP_SETUP(s) m_nameOfComponent = s; entity = e; m_ownerName = e->entityName; input = e->input;

#include <Engine/Components/Creation/CompID.h>
#include "Creation/ComponentFactory.h"

#include <iostream>
#define REGISTER_COMPONENT(T) namespace { struct T##_AutoRegister { T##_AutoRegister() { ComponentFactory::Register( T##_ID, [](WEngine::Entity* e) { return static_cast<T*>(WAllocator::Construct<T>(e)); } ); } }; static T##_AutoRegister global_##T##_AutoRegister; }