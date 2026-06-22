#include "Viewport.h"

#include <cstdint>

#include "Editor/Types/EditorState.h"
#include "Engine/Core/Handlers/RenderHandler.h"
#include "Engine/Core/System/Iris.h"
#include "Engine/Types/CoreSystems.h"
#include <Engine/Components/Rendering/CameraComponent.h>
#include <Engine/Core/World/Entity.h>

using namespace WEditor;

void Viewport::Setup()
{
    m_widgetName = "Viewport";

    m_viewportEntity =  WAllocator::Construct<WEngine::Entity>();
    m_viewportEntity->transform.size = WEngine::Vector3::One;
    viewportCam = WAllocator::Construct<WEngine::CameraComponent>(m_viewportEntity);
    viewportCam->Start();
    //WEngine::CoreSystems::GetRenderHandler()->SetNewCamera(viewportCam);
}

void Viewport::RenderInternal()
{
    // due to docking and resolutions, we need to do this
    ImVec2 size = ImGui::GetContentRegionAvail();

    viewportCam->Tick(0.0f);

    WEngine::Framebuffer fb = WEngine::CoreSystems::GetRenderHandler()->EditorGetViewportFramebuffer();
    auto textureNullable = Iris::FramebufferToImGui(fb);
//
    //if (textureNullable.HasValue())
    //{
    //    ImGui::Image(
    //        textureNullable.GetValue(),
    //        size,
    //        ImVec2(0, 1), ImVec2(1, 0) // flip vertically due to weird stuffs
    //    );
    //}
    //else
    //{
    //    ImGui::Text("Viewport is currently unavailable :(");
    //}
//
    //EditorState::ViewportSelected = ImGui::IsWindowFocused();
    //ImVec2 winSize = ImGui::GetWindowSize();
    //CheckForSizeChange(winSize);
}

void Viewport::CheckForSizeChange(ImVec2 newRes)
{
    // idk why, but imgui returns a really small value, then the
    // real one, to here we check if its higher than 100
    //if (newRes.x > 100 && newRes.y > 100 && !m_resDecided)
    //{
    //    WEngine::CoreSystems::GetRenderHandler()->ResizeViewport(WEngine::Vector2(newRes.x, newRes.y));
    //    m_resDecided = true;
    //}
}
