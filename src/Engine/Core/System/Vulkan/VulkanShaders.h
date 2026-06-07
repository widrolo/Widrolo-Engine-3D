#pragma once

#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include <vulkan/vulkan.h>
#include "VulkanContext.h"
#include "Engine/Types/AssetMission.h"

VkShaderModule CompileShader(const VulkanContext& ctx, const WEngine::SpirVAssetMission& spirvAssetMission);

VkPipelineInputAssemblyStateCreateInfo CreatePipeline_InputAssembly();
VkPipelineVertexInputStateCreateInfo CreatePipeline_VertexDefinition();
VkPipelineShaderStageCreateInfo CreatePipeline_ShaderStange_Vertex(const VulkanContext& ctx, const std::string &shaderName);
VkPipelineShaderStageCreateInfo CreatePipeline_ShaderStange_Fragment(const VulkanContext& ctx, const std::string &shaderName);

VkPipeline CreatePipeline(VulkanContext& ctx, VkRenderPass renderPass, const std::string& shaderName);
void SaturateDescriptorSet(VulkanContext& ctx, Vulkan_Material& material);
#endif
