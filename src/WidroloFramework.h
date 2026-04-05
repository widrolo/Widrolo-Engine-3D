#pragma once

// This file should be included in every game logic file

#include <Engine/Core/System/Memory.h>

// Components
#include <Engine/Components/InputComponent.h>
#include <Engine/Components/Rendering/SpriteRendererComponent.h>
#include <Engine/Components/Rendering/DebugRendererComponent.h>
#include <Engine/Components/Rendering/CameraComponent.h>
#include <Engine/Components/Rendering/AnimationRendererComponent.h>
#include <Engine/Components/SectorComponent.h>
#include <Engine/Components/Physics/StaticBody.h>
#include <Engine/Components/Physics/DynamicBody.h>
#include <Engine/Components/RNGComponent.h>
#include <Engine/Components/AudioComponent.h>

// Data Storage
#include <Engine/Types/Rendering/Sprite.h>
#include <Engine/Types/AssetMission.h>
#include <Engine/Types/Rendering/Animation.h>
#include <Engine/Types/Audio.h>
#include <Engine/Types/Nullable.h>
#include <Engine/Types/CommonTypes.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/Types/Input/KeyCodes.h>
#include <Engine/Math/Math.h>
#include <Engine/Math/Transform.h>
#include <Engine/Types/Input/KeyEvents.h>
#include <Engine/Types/Rendering/Color.h>
#include <Engine/Types/SpawnArgs.h>
#include <Engine/Types/Rendering/RenderMission.h>
#include <Engine/Types/Rendering/GPU/Shader.h>
#include <Engine/Types/Version.h>
#include <Engine/Core/Widget.h>

// Utils 
#include <Engine/Util/Log.h>
#include <Engine/Util/BitwiseMacros.h>

// etc.
#include <Engine/Core/World/Sector.h>
#include <Engine/Core/World/Entity.h>
#include <Engine/UI/UIParser.h>

#include <Engine/Stores/Steam/SteamStore.h>