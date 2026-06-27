#pragma once

#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include <vulkan/vulkan.h>
#include "VulkanContext.h"
#include "VulkanStatistics.h"
#include "Engine/Types/AssetMission.h"
#include "Engine/Types/Rendering/MaterialDefinition.h"
#include "Engine/Types/Rendering/ShaderDefinition.h"

VkShaderModule CompileShader(const VulkanContext& ctx, const WEngine::SpirVAssetMission& spirvAssetMission);

VkPipelineInputAssemblyStateCreateInfo CreatePipeline_InputAssembly();
VkPipelineVertexInputStateCreateInfo CreatePipeline_VertexDefinition();
VkPipelineShaderStageCreateInfo CreatePipeline_ShaderStange_Vertex(const VulkanContext& ctx, const std::string &shaderName);
VkPipelineShaderStageCreateInfo CreatePipeline_ShaderStange_Fragment(const VulkanContext& ctx, const std::string &shaderName);

VkPipeline CreatePipeline(VulkanContext& ctx, const WEngine::ShaderDefinition& shaderDef, VkPipelineLayout pipelineLayout);
void SaturateDescriptorSet(VulkanContext& ctx, Vulkan_Material& material);

void TryCompileAllShaders(VulkanContext& ctx);
WEngine::Material CompileMaterial(VulkanContext& ctx, const std::string& matName);

void UpdateLighting(VulkanContext& ctx);

#endif
