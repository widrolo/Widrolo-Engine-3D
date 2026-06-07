#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "VulkanPipeline.h"
#include <vulkan/vulkan.h>
#include "Engine/imgui/backends/imgui_impl_vulkan.h"

#include "VulkanHelpers.h"
#include "Engine/Math/Matrix.h"
#include "Engine/Util/Log.h"

bool SetupPipelineLayout(VulkanContext& ctx)
{
    VkPushConstantRange pushConstant{};
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(WEngine::Mat4x4);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &ctx.descSetLayout;

    auto resLayout = vkCreatePipelineLayout(ctx.vcore.gpuDevice, &pipelineLayoutInfo, ctx.vcore.allocator, &ctx.pipelineLayout);
    if (!ParseVkResult(resLayout))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create pipeline layout!");
        return false;
    }
    return true;
}

bool SetupDescriptorSetLayout(VulkanContext &ctx)
{
    VkDescriptorSetLayout layout{};

    VkDescriptorSetLayoutBinding binding{};
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.binding = 0;

    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = &binding;

    auto res = vkCreateDescriptorSetLayout(ctx.vcore.gpuDevice, &info, ctx.vcore.allocator, &layout);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create descriptor set layout");
        return false;
    }

    ctx.descSetLayout = layout;

    return true;
}

VkDescriptorSet CreateDescriptorSet(VulkanContext &ctx)
{
    VkDescriptorSet set;
    VkDescriptorSetAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool = ctx.renderDescriptorPool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &ctx.descSetLayout;

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
    // So far im only using this for imgui, so please adjust when needed.

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

bool SetupRenderDescriptorPool(VulkanContext &ctx)
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = ctx.screen.swapchainImages.size();

    VkDescriptorPoolCreateInfo descInfo{};
    descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descInfo.maxSets = 512;
    descInfo.poolSizeCount = 1;
    descInfo.pPoolSizes = &poolSize;

    auto res = vkCreateDescriptorPool(ctx.vcore.gpuDevice, &descInfo, ctx.vcore.allocator, &ctx.renderDescriptorPool);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create descriptor pool!");
        return false;
    }
    return true;
}

bool SetupCommandPool(VulkanContext& ctx)
{
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = ctx.queues.primaryDrawQueueFamilyIndex;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    auto res = vkCreateCommandPool(ctx.vcore.gpuDevice, &commandPoolInfo, ctx.vcore.allocator, &ctx.commandPool);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create command buffer!");
        return false;
    }
    return true;
}

VkCommandBuffer CreateCommandBuffer(const VulkanContext& ctx, VkCommandPool cmdPool)
{
    VkCommandBuffer cmdBuff;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = cmdPool;
    allocInfo.commandBufferCount = 1;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    auto res = vkAllocateCommandBuffers(ctx.vcore.gpuDevice, &allocInfo, &cmdBuff);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create command buffer!");
        return VK_NULL_HANDLE;
    }

    return cmdBuff;
}

VkRenderPass CreateBasicRenderPass(VulkanContext& ctx)
{
    VkRenderPass renderPass;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = FindBestColorFormat(ctx);
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindBestDepthFormat(ctx);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    auto res = vkCreateRenderPass(ctx.vcore.gpuDevice, &renderPassInfo, ctx.vcore.allocator, &renderPass);
    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create command buffer!");
        return VK_NULL_HANDLE;
    }
    return renderPass;
}

#endif

