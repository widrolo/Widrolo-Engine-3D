cmake_minimum_required(VERSION 4.0)
project(Widrolo_Engine_3D)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Packaging" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(SDL_PATH        "${CMAKE_SOURCE_DIR}/libs/SDL3-3.2.24")
set(STEAMWORKS_PATH "${CMAKE_SOURCE_DIR}/libs/Steamworks-SDK")
set(CURL_PATH       "${CMAKE_SOURCE_DIR}/libs/curl-8.13.0")
set(YAMLCPP_PATH    "${CMAKE_SOURCE_DIR}/libs/yaml-cpp-0.8.0")
set(GLM_PATH        "${CMAKE_SOURCE_DIR}/libs/glm-1.0.1")
set(TINYXML2_PATH   "${CMAKE_SOURCE_DIR}/libs/tinyxml2-11.0.0")

# thank god for CLion
add_executable(Widrolo_Engine_3D
        src/Engine/Components/Physics/DynamicBody.cpp
        src/Engine/Components/Physics/DynamicBody.h
        src/Engine/Components/Physics/StaticBody.cpp
        src/Engine/Components/Physics/StaticBody.h
        src/Engine/Components/Rendering/AnimationRendererComponent.cpp
        src/Engine/Components/Rendering/AnimationRendererComponent.h
        src/Engine/Components/Rendering/CameraComponent.cpp
        src/Engine/Components/Rendering/CameraComponent.h
        src/Engine/Components/Rendering/DebugRendererComponent.cpp
        src/Engine/Components/Rendering/DebugRendererComponent.h
        src/Engine/Components/Rendering/SpriteRendererComponent.cpp
        src/Engine/Components/Rendering/SpriteRendererComponent.h
        src/Engine/Components/AudioComponent.cpp
        src/Engine/Components/AudioComponent.h
        src/Engine/Components/Component.cpp
        src/Engine/Components/Component.h
        src/Engine/Components/InputComponent.cpp
        src/Engine/Components/InputComponent.h
        src/Engine/Components/RNGComponent.cpp
        src/Engine/Components/RNGComponent.h
        src/Engine/Components/SectorComponent.cpp
        src/Engine/Components/SectorComponent.h
        src/Engine/Core/Handlers/AssetRepo.cpp
        src/Engine/Core/Handlers/AssetRepo.h
        src/Engine/Core/Handlers/AudioHandler.cpp
        src/Engine/Core/Handlers/AudioHandler.h
        src/Engine/Core/Handlers/InputHandler.cpp
        src/Engine/Core/Handlers/InputHandler.h
        src/Engine/Core/Handlers/PhysicsHandler.cpp
        src/Engine/Core/Handlers/PhysicsHandler.h
        src/Engine/Core/Handlers/RenderHandler.cpp
        src/Engine/Core/Handlers/RenderHandler.h
        src/Engine/Core/Handlers/RNGHandler.cpp
        src/Engine/Core/Handlers/RNGHandler.h
        src/Engine/Core/Engine.cpp
        src/Engine/Core/Engine.h
        src/Engine/Core/World/Entity.cpp
        src/Engine/Core/World/Entity.h
        src/Engine/Core/World/Sector.cpp
        src/Engine/Core/World/Sector.h
        src/Engine/Core/World/SectorLogic.cpp
        src/Engine/Core/World/SectorLogic.h
        src/Engine/gl/include/glad/glad.h
        src/Engine/gl/include/KHR/khrplatform.h
        src/Engine/gl/include/stb_image.cpp
        src/Engine/gl/include/stb_image.h
        src/Engine/gl/src/glad.c
        src/Engine/EngineWidgets/StatisticsWidgets.cpp
        src/Engine/EngineWidgets/StatisticsWidgets.h
        src/Engine/EngineWidgets/TimingsWidget.cpp
        src/Engine/EngineWidgets/TimingsWidget.h
        src/Engine/imgui/backends/imgui_impl_opengl3.cpp
        src/Engine/imgui/backends/imgui_impl_opengl3.h
        src/Engine/imgui/backends/imgui_impl_opengl3_loader.h
        src/Engine/imgui/backends/imgui_impl_sdl3.cpp
        src/Engine/imgui/backends/imgui_impl_sdl3.h
        src/Engine/imgui/imconfig.h
        src/Engine/imgui/imgui.cpp
        src/Engine/imgui/imgui.h
        src/Engine/imgui/imgui_demo.cpp
        src/Engine/imgui/imgui_draw.cpp
        src/Engine/imgui/imgui_internal.h
        src/Engine/imgui/imgui_tables.cpp
        src/Engine/imgui/imgui_widgets.cpp
        src/Engine/imgui/implot.cpp
        src/Engine/imgui/implot.h
        src/Engine/imgui/implot_demo.cpp
        src/Engine/imgui/implot_internal.h
        src/Engine/imgui/implot_items.cpp
        src/Engine/imgui/imstb_rectpack.h
        src/Engine/imgui/imstb_textedit.h
        src/Engine/imgui/imstb_truetype.h
        src/Engine/Core/System/OS.cpp
        src/Engine/Core/System/OS.h
        src/Engine/Stores/Steam/SteamStore.cpp
        src/Engine/Stores/Steam/SteamStore.h
        src/Engine/Types/Rendering/Animation.h
        src/Engine/Types/AssetMission.h
        src/Engine/Types/Rendering/Atlas.cpp
        src/Engine/Types/Rendering/Atlas.h
        src/Engine/Types/Audio.h
        src/Engine/Types/Rendering/Color.h
        src/Engine/Types/CommonTypes.h
        src/Engine/Types/CoreSystems.h
        src/Engine/Types/Input/KeyCodes.h
        src/Engine/Types/Input/KeyEvents.h
        src/Engine/Math/Math.h
        src/Engine/Types/Nullable.h
        src/Engine/Types/Rendering/RenderMission.h
        src/Engine/Types/Rendering/GPU/Shader.h
        src/Engine/Types/SpawnArgs.cpp
        src/Engine/Types/SpawnArgs.h
        src/Engine/Types/Rendering/Sprite.cpp
        src/Engine/Types/Rendering/Sprite.h
        src/Engine/Math/Transform.cpp
        src/Engine/Math/Transform.h
        src/Engine/Types/Version.cpp
        src/Engine/Types/Version.h
        src/Engine/Core/Widget.cpp
        src/Engine/Core/Widget.h
        src/Engine/Util/BitwiseMacros.h
        src/Engine/Util/Conversions.h
        src/Engine/Util/Env.h
        src/Engine/Util/Log.cpp
        src/Engine/Util/Log.h
        src/Engine/EngineDefines.h
        src/Engine/Entry-Point.cpp
        src/Engine/Components/Creation/ComponentFactory.cpp
        src/Engine/Components/Creation/ComponentFactory.h
        src/Game/GameDefines.h
        src/WGL.h
        src/WidroloFramework.h
        src/Engine/Components/Creation/CompID.h
        src/Engine/Math/Vector.h
        src/Engine/Math/Shapes.h
        src/Engine/Math/Matrix.h
        src/Engine/Math/Vectors/Vectors.cpp
        src/Engine/Math/Vectors/VecMath.cpp
        src/Engine/Math/Other/Quaternion.h
        src/Engine/Math/Other/Quaternion.cpp
        src/Engine/Math/Matrices/MatrixMath.cpp
        src/Engine/Math/Functions/Trigonometry.cpp
        src/Engine/Math/Functions/Power.cpp
        src/Engine/Util/Time.cpp
        src/Engine/Util/Time.h
        src/Engine/Types/Input/InputDevice.h
        src/Engine/EngineWidgets/PeripheralWidget.cpp
        src/Engine/EngineWidgets/PeripheralWidget.h
        src/Engine/Types/Input/KeyCodes.cpp
        src/Engine/Types/Input/InputDevice.cpp
        src/Engine/Core/Handlers/WidgetHandler.cpp
        src/Engine/Core/Handlers/WidgetHandler.h
        src/Engine/EngineWidgets/SystemWidget.cpp
        src/Engine/EngineWidgets/SystemWidget.h
        src/Engine/EngineWidgets/GameSystemWidget.cpp
        src/Engine/EngineWidgets/GameSystemWidget.h
        src/Engine/EngineWidgets/SectorWatchWidget.h
        src/Engine/EngineWidgets/SectorWatchWidget.cpp
        src/Engine/EngineWidgets/EngineControlWidget.cpp
        src/Engine/EngineWidgets/EngineControlWidget.h
        src/Engine/EngineWidgets/PhysicsWatchWidget.cpp
        src/Engine/EngineWidgets/PhysicsWatchWidget.h
        src/Engine/Core/System/Memory.cpp
        src/Engine/Core/System/Memory.h
        src/Engine/WTL/vector.h
        src/Engine/WTL/allocator.h
        src/Engine/WTL/deque.h
        src/Engine/UI/UIWidget.cpp
        src/Engine/UI/UIWidget.h
        src/Engine/UI/UIElement.h
        src/Engine/UI/Elements/Buttons/Button.cpp
        src/Engine/UI/Elements/Buttons/Button.h
        src/Engine/UI/Elements/Containers/Container.cpp
        src/Engine/UI/Elements/Containers/Container.h
        src/Engine/UI/Elements/Dropdowns/Dropdown.cpp
        src/Engine/UI/Elements/Dropdowns/Dropdown.h
        src/Engine/UI/Elements/Images/Image.cpp
        src/Engine/UI/Elements/Images/Image.h
        src/Engine/UI/Elements/InputFields/TextInput.cpp
        src/Engine/UI/Elements/InputFields/TextInput.h
        src/Engine/UI/Elements/Labels/TextLabel.cpp
        src/Engine/UI/Elements/Labels/TextLabel.h
        src/Engine/UI/Elements/Seperators/Seperator.cpp
        src/Engine/UI/Elements/Seperators/Seperator.h
        src/Engine/UI/Elements/Sliders/Slider.cpp
        src/Engine/UI/Elements/Sliders/Slider.h
        src/Engine/UI/Elements/Toggles/Checkbox.cpp
        src/Engine/UI/Elements/Toggles/Checkbox.h
        src/Engine/WTL/queue.h
        src/Engine/Types/Rendering/Color.cpp
        src/Engine/UI/UIParser.cpp
        src/Engine/UI/UIParser.h
        src/Engine/Core/Handlers/JobHandler.cpp
        src/Engine/Core/Handlers/JobHandler.h
        src/Engine/Types/Jobs/WorkerThreadState.h
        src/Engine/Core/System/ThreadScheduler.cpp
        src/Engine/Core/System/ThreadScheduler.h
        src/Engine/Types/Jobs/Job.h
        src/Engine/Types/Jobs/JobLengthEstimation.h
        src/Engine/Core/Physics/PhysicsSolver.cpp
        src/Engine/Core/Physics/PhysicsSolver.h
        src/Engine/Core/Physics/SimulatableObject.h
        src/Engine/Core/Physics/SimulatableObject.cpp
        src/Engine/Core/Physics/PhysicsArea.h
        src/Engine/Components/Physics/CircleAreaComponent.cpp
        src/Engine/Components/Physics/CircleAreaComponent.h
        src/Engine/Components/Physics/RectAreaComponent.cpp
        src/Engine/Components/Physics/RectAreaComponent.h
        src/Engine/Components/Physics/BoundaryAreaComponent.cpp
        src/Engine/Components/Physics/BoundaryAreaComponent.h
        src/Engine/Components/Physics/CylinderAreaComponent.cpp
        src/Engine/Components/Physics/CylinderAreaComponent.h
        src/Engine/Types/Physics/OverlapResult.h
        src/Engine/WTL/map.h
        src/Engine/Core/Physics/PhysicsVisualizer.cpp
        src/Engine/Core/Physics/PhysicsVisualizer.h
        src/Editor/EditorDefines.h
        src/Editor/Core/Editor.cpp
        src/Editor/Core/Editor.h
        src/Editor/Components/AnyComponent.cpp
        src/Editor/Components/AnyComponent.h
        src/Editor/Types/EditorSystems.h
        src/Editor/Core/Handlers/EditorUIHandler.cpp
        src/Editor/Core/Handlers/EditorUIHandler.h
        src/Editor/EditorWidgets/AboutWidget.cpp
        src/Editor/EditorWidgets/AboutWidget.h
        src/Editor/Core/Handlers/MenubarHandler.cpp
        src/Editor/Core/Handlers/MenubarHandler.h
        src/Editor/Types/MenubarItem.h
        src/Editor/MenubarItems/File/Menubar_Exit.cpp
        src/Editor/MenubarItems/File/Menubar_Exit.h
        src/Editor/Types/ComponentSettingDefinition.h
        src/Editor/Core/Handlers/CompSettingsRepo.cpp
        src/Editor/Core/Handlers/CompSettingsRepo.h
        src/Editor/MenubarItems/File/Menubar_RunGame.cpp
        src/Editor/MenubarItems/File/Menubar_RunGame.h
        src/Editor/EditorWidgets/SectorList.cpp
        src/Editor/EditorWidgets/SectorList.h
        src/Editor/EditorWidgets/EntityList.cpp
        src/Editor/EditorWidgets/EntityList.h
        src/Editor/EditorWidgets/ComponentList.cpp
        src/Editor/EditorWidgets/ComponentList.h
        src/Editor/EditorWidgets/ComponentSettings.cpp
        src/Editor/EditorWidgets/ComponentSettings.h
        src/Editor/EditorWidgets/Viewport.cpp
        src/Editor/EditorWidgets/Viewport.h
        src/Editor/MenubarItems/Help/Menubar_About.cpp
        src/Editor/MenubarItems/Help/Menubar_About.h
        src/Editor/Types/EditorState.h
        src/Editor/Components/EditorCompFactory.cpp
        src/Editor/Components/EditorCompFactory.h
        src/Game/Core/Game.cpp
        src/Game/Core/Game.h
        src/Engine/WTL/list.h
        src/Editor/EditorWidgets/Tools/ResolutionFinderWidget.cpp
        src/Editor/EditorWidgets/Tools/ResolutionFinderWidget.h
        src/Editor/MenubarItems/Tools/Menubar_ResFind.cpp
        src/Editor/MenubarItems/Tools/Menubar_ResFind.h
        src/Engine/Core/System/GPU_OpenGL.cpp
        src/Engine/Core/System/GPU.h
        src/Engine/Types/Rendering/GPU/Framebuffer.h
        src/Engine/Types/Rendering/ShaderSettings.h
        src/Engine/Types/Rendering/GPU/Texture.h
        src/Engine/Types/Rendering/GPU/Model.h
        src/Engine/Types/Rendering/ModelInfo.h
        src/Engine/Types/Rendering/GPU/Line.h
        src/Engine/Types/Rendering/LineInfo.h
        src/Engine/Types/Rendering/TextureInfo.h
        src/Engine/Core/System/GPU_Vulkan.cpp
        src/Engine/Types/DebugFlags.h
        src/Engine/EngineWidgets/DebugFlagsWidget.cpp
        src/Engine/EngineWidgets/DebugFlagsWidget.h
        src/Editor/EditorWidgets/Tools/AnimationEditorWidget.cpp
        src/Editor/EditorWidgets/Tools/AnimationEditorWidget.h
        src/Editor/MenubarItems/Tools/Menubar_AnimEditor.cpp
        src/Editor/MenubarItems/Tools/Menubar_AnimEditor.h
        src/Engine/Types/Rendering/Animation.cpp
        src/Engine/Types/Rendering/PostProcessShader.h
        src/Engine/imgui/crude_json.cpp
        src/Engine/imgui/imgui_bezier_math.h
        src/Engine/imgui/imgui_bezier_math.inl
        src/Engine/imgui/imgui_canvas.cpp
        src/Engine/imgui/imgui_canvas.h
        src/Engine/imgui/imgui_extra_math.h
        src/Engine/imgui/imgui_extra_math.inl
        src/Engine/imgui/imgui_node_editor.cpp
        src/Engine/imgui/imgui_node_editor.h
        src/Engine/imgui/imgui_node_editor_api.cpp
        src/Engine/imgui/imgui_node_editor_internal.h
        src/Engine/imgui/imgui_node_editor_internal.inl
        src/Editor/MenubarItems/Help/Menubar_NodeTest.cpp
        src/Editor/MenubarItems/Help/Menubar_NodeTest.h
        src/Editor/EditorWidgets/Tests/NodeTestWidget.cpp
        src/Editor/EditorWidgets/Tests/NodeTestWidget.h
        src/Engine/Types/AI/Behaviour/BehaviourTree.h
        src/Engine/Types/AI/Behaviour/BehaviourNode.h
        src/Engine/Types/AI/Behaviour/BehaviourExecutionStatus.h
        src/Engine/Types/Blackboard.h
        src/Engine/Types/BlackboardVariable.h
        src/Engine/Types/BlackboardVariable.cpp
        src/Engine/Types/AI/Behaviour/BehaviourNode.cpp
        src/Engine/Types/AI/Behaviour/Control/SelectorNode.cpp
        src/Engine/Types/AI/Behaviour/Control/SelectorNode.h
        src/Engine/Types/AI/Behaviour/Control/ParallelNode.cpp
        src/Engine/Types/AI/Behaviour/Control/ParallelNode.h
        src/Engine/Types/AI/Behaviour/Control/SequenceNode.cpp
        src/Engine/Types/AI/Behaviour/Control/SequenceNode.h
        src/Engine/Types/AI/Behaviour/Control/InverterNode.cpp
        src/Engine/Types/AI/Behaviour/Control/InverterNode.h
        src/Engine/Types/AI/Behaviour/BehaviourTree.cpp
        src/Engine/Types/AI/Behaviour/Control/BranchNode.cpp
        src/Engine/Types/AI/Behaviour/Control/BranchNode.h
        src/Engine/Core/System/GPUSettings.h
        src/Engine/imgui/backends/imgui_impl_vulkan.cpp
)

set_target_properties(Widrolo_Engine_3D PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin/${CMAKE_SYSTEM_NAME}-x64"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin/${CMAKE_SYSTEM_NAME}-x64"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin/${CMAKE_SYSTEM_NAME}-x64"
)

target_include_directories(Widrolo_Engine_3D PRIVATE
        src
        ${SDL_PATH}/include
        ${STEAMWORKS_PATH}/include
        ${CURL_PATH}/include
        ${YAMLCPP_PATH}/include
        ${GLM_PATH}/include
        ${TINYXML2_PATH}/include
        src/Engine/gl/include # glad
)

# ---- Common defines ----
target_compile_definitions(Widrolo_Engine_3D PRIVATE
        _CRT_SECURE_NO_WARNINGS
        YAML_CPP_STATIC_DEFINE
)

#add_compile_options(-fsized-deallocation)

if(MSVC)
    target_compile_options(Widrolo_Engine_3D PRIVATE /MP /wd4005)
endif()

if(WIN32)
    message(STATUS Detected Windows)
    target_compile_definitions(Widrolo_Engine_3D PRIVATE WE_Windows)

    target_link_libraries(Widrolo_Engine_3D PRIVATE
            ${STEAMWORKS_PATH}/lib/steam_api64.lib
            ${SDL_PATH}/lib/SDL3.lib
            ${CURL_PATH}/lib/libcurl.lib
    )

    target_link_libraries(Widrolo_Engine_3D PRIVATE
            $<$<CONFIG:Debug>:${YAMLCPP_PATH}/lib/yaml-cppd.lib>
            $<$<CONFIG:Release>:${YAMLCPP_PATH}/lib/yaml-cpp.lib>
            $<$<CONFIG:Packaging>:${YAMLCPP_PATH}/lib/yaml-cpp.lib>
            $<$<CONFIG:Debug>:${TINYXML2_PATH}/lib/tinyxml2d.lib>
            $<$<CONFIG:Release>:${TINYXML2_PATH}/lib/tinyxml2.lib>
            $<$<CONFIG:Packaging>:${TINYXML2_PATH}/lib/tinyxml2.lib>
    )

    # Config-specific link rules
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Widrolo_Engine_3D)

else() # really should be elseif(linux), but it doesnt work...
    message(STATUS Detected Linux)
    target_compile_definitions(Widrolo_Engine_3D PRIVATE WE_Linux)
    target_link_libraries(Widrolo_Engine_3D PRIVATE
            ${SDL_PATH}/lib/libSDL3.a
            #${CURL_PATH}/lib/libcurl.a
            ${YAMLCPP_PATH}/lib/libyaml-cpp.a
            ${TINYXML2_PATH}/lib/libtinyxml2.a
            ${STEAMWORKS_PATH}/lib/libsteam_api.so
            GL
            dl
            pthread
            vulkan
    )
endif()

set_property(TARGET Widrolo_Engine_3D APPEND PROPERTY
        COMPILE_DEFINITIONS $<$<CONFIG:Debug>:DEBUG>
)

# RelCanditate
set_property(TARGET Widrolo_Engine_3D APPEND PROPERTY
        COMPILE_DEFINITIONS $<$<CONFIG:Release>:NDEBUG;RC>
)

# Packaging
set_property(TARGET Widrolo_Engine_3D APPEND PROPERTY
        COMPILE_DEFINITIONS $<$<CONFIG:Packaging>:NDEBUG;PACKAGE>
)

set_target_properties(Widrolo_Engine_3D PROPERTIES
        WIN32_EXECUTABLE $<$<CONFIG:Packaging>:TRUE>
        OUTPUT_NAME "$<$<CONFIG:Debug>:Widrolo_Engine_3D-Dev>$<$<CONFIG:Release>:Widrolo_Engine_3D-RC>$<$<CONFIG:Packaging>:Widrolo_Engine_3D-Package>"
)