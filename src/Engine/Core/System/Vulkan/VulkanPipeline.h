#pragma once

#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "VulkanContext.h"
#include "Engine/Types/Rendering/ShaderDefinition.h"

VkPipelineLayout CreatePipelineLayout(VulkanContext& ctx, VkDescriptorSetLayout descLayout);
VkDescriptorSetLayout CreateDescriptorSetLayout(VulkanContext& ctx, const WEngine::ShaderDefinition& shaderDef);
VkDescriptorSet CreateDescriptorSet(VulkanContext& ctx, const Vulkan_Shader& shader);
bool SetupImGuiDescriptorPool(VulkanContext& ctx);
VkDescriptorPool CreateDescriptorPool(VulkanContext& ctx, const WEngine::ShaderDefinition& shaderDef);
bool SetupCommandPool(VulkanContext& ctx);
VkCommandBuffer CreateCommandBuffer(const VulkanContext& ctx, VkCommandPool cmdPool);
VkRenderPass CreateBasicRenderPass(VulkanContext& ctx);

#endif