#include "EditorCompFactory.h"

#include "AnyComponent.h"
#include <Engine/Types/SpawnArgs.h>

#include "Editor/Types/EditorSystems.h"
#include <Editor/Core/Handlers/CompSettingsRepo.h>

WEngine::Component* EditorCompFactory::CreateComponent(const WEngine::ComponentArgs& args, WEngine::Entity* e)
{
    WEditor::AnyComponent* component = nullptr;

    component = new WEditor::AnyComponent(e);

    if (component != nullptr)
    {
        int capacity = WEditor::EditorSystems::GetCompSettingsRepo()->GetSettingCapacity(args.componentTypeId);
        component->Init(args.componentTypeId, capacity, args);
    }

    return component;
}
