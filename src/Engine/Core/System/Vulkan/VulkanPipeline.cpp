#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "VulkanPipeline.h"
#include <vulkan/vulkan.h>
#include "Engine/imgui/backends/imgui_impl_vulkan.h"

#include "VulkanHelpers.h"
#include "Engine/Core/System/GPUSettings.h"
#include "Engine/Math/Matrix.h"
#include "Engine/Util/Log.h"

VkPipelineLayout CreatePipelineLayout(VulkanContext& ctx, VkDescriptorSetLayout descLayout)
{
    std::array<VkPushConstantRange, 2> pushConstants;

    // mvp / vp
    pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstants[0].offset = 0;
    pushConstants[0].size = sizeof(WEngine::Mat4x4);

    struct FragmentConstants
    {
        alignas(16) WEngine::Vector3 sunDir;
        alignas(16) WEngine::Vector3 camPos;
    };

    pushConstants[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstants[1].offset = sizeof(WEngine::Mat4x4);
    pushConstants[1].size = sizeof(FragmentConstants);


    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();
    pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

    if (descLayout != VK_NULL_HANDLE)
    {
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descLayout;
    }

    VkPipelineLayout layout;

    auto resLayout = vkCreatePipelineLayout(ctx.vcore.gpuDevice, &pipelineLayoutInfo, ctx.vcore.allocator, &layout);
    if (!ParseVkResult(resLayout))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create pipeline layout!");
        return VK_NULL_HANDLE;
    }
    return layout;
}

VkDescriptorSetLayout CreateDescriptorSetLayout(VulkanContext &ctx, const WEngine::ShaderDefinition& shaderDef)
{
    VkDescriptorSetLayout layout{};

    wtl::vector<VkDescriptorSetLayoutBinding> bindings(shaderDef.fragInfo.expectTextureCount);

    for (uint32_t i = 0; i < bindings.size(); ++i)
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1;
        binding.binding = i;
        bindings[i] = binding;
    }

    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = bindings.size();
    info.pBindings = bindings.data();

    auto res = vkCreateDescriptorSetLayout(ctx.vcore.gpuDevice, &info, ctx.vcore.allocator, &layout);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create descriptor set layout");
        return VK_NULL_HANDLE;
    }

    return layout;
}

VkDescriptorSet CreateDescriptorSet(VulkanContext &ctx, const Vulkan_Shader& shader)
{
    VkDescriptorSet set;
    VkDescriptorSetAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool = shader.descriptorPool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &shader.descriptorSetLayout;

    auto res = vkAllocateDescriptorSets(ctx.vcore.gpuDevice, &info, &set);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create descriptor set!");
        return VK_NULL_HANDLE;
    }
    return set;
}

bool SetupImGuiDescriptorPool(VulkanContext& ctx)
{
    wtl::vector<VkDescriptorPoolSize> poolSizes =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE},
        { VK_DESCRIPTOR_TYPE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
    };
    VkDescriptorPoolCreateInfo descInfo{};
    descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descInfo.maxSets = 0;
    for (VkDescriptorPoolSize& pool_size : poolSizes)
        descInfo.maxSets += pool_size.descriptorCount;
    descInfo.poolSizeCount = poolSizes.size();
    descInfo.pPoolSizes = poolSizes.data();

    auto res = vkCreateDescriptorPool(ctx.vcore.gpuDevice, &descInfo, ctx.vcore.allocator, &ctx.imGuiDescriptorPool);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create descriptor pool!");
        return false;
    }
    return true;
}

VkDescriptorPool CreateDescriptorPool(VulkanContext &ctx, const WEngine::ShaderDefinition& shaderDef)
{
    if (shaderDef.fragInfo.expectTextureCount == 0 && shaderDef.fragInfo.expectedParams.empty())
    {
        WEngine::WLog::SetConsoleInfo();
        WEngine::WLog::ConsoleLog(std::format("Shader \"{}\" needs no descriptor set.", shaderDef.name));
        return VK_NULL_HANDLE;
    }

    uint16 maxSets = 0;
    switch (shaderDef.abundance)
    {
        case 0: maxSets = 4; break;
        case 1: maxSets = 128; break;
        case 2: maxSets = 512; break;
        case 3: maxSets = 2048; break;
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = shaderDef.fragInfo.expectTextureCount * maxSets;

    VkDescriptorPoolCreateInfo descInfo{};
    descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descInfo.maxSets = maxSets;
    descInfo.poolSizeCount = 1;
    descInfo.pPoolSizes = &poolSize;

    VkDescriptorPool descriptorPool{};

    auto res = vkCreateDescriptorPool(ctx.vcore.gpuDevice, &descInfo, ctx.vcore.allocator, &descriptorPool);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create descriptor pool!");
        return VK_NULL_HANDLE;
    }
    return descriptorPool;
}



#endif

