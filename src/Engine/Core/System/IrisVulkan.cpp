#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "Iris.h"

#include <Engine/EngineDefines.h>
#include <Game/GameDefines.h>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "Engine/imgui/imgui.h"
#include "Engine/imgui/backends/imgui_impl_sdl3.h"
#include "Engine/imgui/backends/imgui_impl_vulkan.h"

#include "GPUSettings.h"
#include "Engine/Util/BitwiseMacros.h"
#include "Engine/Util/Log.h"

#include <Engine/Types/Rendering/VertextData.h>

#include "Engine/Core/Handlers/AssetRepo.h"
#include "Engine/Types/CoreSystems.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "Engine/Types/Rendering/InstanceData.h"
#include "Engine/Types/Rendering/Iris/InstThreadedList.h"

// Vulkan implementation subfiles
#include "Vulkan/VulkanTypes.h"
#include "Vulkan/VulkanHelpers.h"
#include "Vulkan/VulkanCore.h"
#include "Vulkan/VulkanQueues.h"
#include "Vulkan/VulkanSwapchain.h"
#include "Vulkan/VulkanPipeline.h"
#include "Vulkan/VulkanShaders.h"
#include "Vulkan/VulkanBuffers.h"
#include "Vulkan/VulkanCommands.h"
#include "Vulkan/VulkanImages.h"

VulkanContext ctx;
VulkanStatistics stats;

bool Iris::SETTING_InitGPUApi(SDL_Window *window)
{
    if (!SetupVkInstance(ctx))
        return false;
#ifndef PACKAGE
    SetupValidation(ctx);
#endif
    if (!SetupGraphicsDevice(ctx))
        return false;
    SetupVmaAllocator(ctx);

    if (!SDL_Vulkan_CreateSurface(window, ctx.vcore.instance, ctx.vcore.allocator, &ctx.screen.screen))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Failed to create a Vulkan surface for the screen, {}", SDL_GetError()));
        return false;
    }

    if (!SetupDepthImage(ctx, stats))
        return false;
    if (!SetupSwapchain(ctx, stats))
        return false;
    if (!SetupDisplayRenderTarget(ctx, stats))
        return false;
    if (!SetupStationaryInstanceBuffer(ctx, stats))
        return false;
    if (!SetupLightingBuffer(ctx, stats))
        return false;
    if (!SetupLightingDescriptors(ctx))
        return false;
    if (!SetupTransferCommandBuffer(ctx))
        return false;

    TryCompileAllShaders(ctx);
    return true;
}

void Iris::SETTING_ConfigureImGui(SDL_Window *window)
{
    if (!SetupImGuiDescriptorPool(ctx))
        return;

    static VkFormat swapFormat = FindBestSwapchainFormat(ctx);

    VkPipelineRenderingCreateInfo pipeInfo{};
    pipeInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipeInfo.colorAttachmentCount = 1;
    pipeInfo.pColorAttachmentFormats = &swapFormat;
    pipeInfo.depthAttachmentFormat = FindBestDepthFormat(ctx);


    ImGui_ImplSDL3_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = GetVulkanVersion();
    initInfo.Instance = ctx.vcore.instance;
    initInfo.PhysicalDevice = ctx.vcore.gpuPhysicalDevice;
    initInfo.Device = ctx.vcore.gpuDevice;
    initInfo.Allocator = ctx.vcore.allocator;
    initInfo.QueueFamily = ctx.queues.primaryDrawQueueFamilyIndex;
    initInfo.Queue = ctx.queues.primaryDrawQueue;
    initInfo.ImageCount = ctx.screen.swapchainImageCount;
    initInfo.MinImageCount = ctx.screen.swapchainImageCount;
    initInfo.DescriptorPoolSize = 8;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipeInfo;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);
}

void Iris::SETTING_BeginNewPreFrame()
{
    BeginTextureUpload(ctx);
}


void Iris::SETTING_BeginNewFrame()
{
    UploadTextures(ctx);

    ctx.currentRenderTarget = &ctx.displayTarget;


    vkWaitForFences(ctx.vcore.gpuDevice, 1, &GetFbEndOfFrameFence(ctx), VK_TRUE, UINT64_MAX);

    for (auto& buf : ctx.bufferGraveyard[ctx.screen.currentFrame])
        vmaDestroyBuffer(ctx.vcore.vmaAllocator, buf.first, buf.second);
    ctx.bufferGraveyard[ctx.screen.currentFrame].clear();

    //vkResetFences(ctx.vcore.gpuDevice, 1, &GetFbEndOfFrameFence(ctx));
    vkAcquireNextImageKHR(ctx.vcore.gpuDevice, ctx.screen.swapchain, max_uint64,
        ctx.displayTarget.imageAvailableSems[ctx.screen.currentFrame], VK_NULL_HANDLE, &ctx.screen.swapchainCurrentImage);

    for (auto& model : ctx.loadedModels)
        model.activeInstances = 0;
}

void Iris::SETTING_SetViewportSize(WEngine::Vector2 size)
{
    VkViewport viewport{};
    viewport.width = size.x;
    viewport.height = size.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(GetFbCmdBuff(ctx), 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent.width = size.x;
    scissor.extent.height = size.y;
    vkCmdSetScissor(GetFbCmdBuff(ctx), 0, 1, &scissor);
}

void Iris::SETTING_SetLighting(const WEngine::LightingInfo &light)
{
    ctx.lighting.lightingInfo = light;
    UpdateLighting(ctx);
}

WEngine::Nullable<WEngine::ShaderDefinition> Iris::GetShaderDef(const std::string &shaderName)
{
    if (!ctx.loadedShadersHandles.contains(shaderName))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Shader \"{}\" not found!", shaderName));
        return WEngine::Nullable<WEngine::ShaderDefinition>();
    }

    WEngine::Shader shHandle = ctx.loadedShadersHandles[shaderName];
    Vulkan_Shader shader = ctx.loadedShaders[shHandle - 1];
    return WEngine::Nullable<WEngine::ShaderDefinition>(shader.shaderDefinition);
}

WEngine::Nullable<WEngine::Material> Iris::GetMaterial(const std::string &matName)
{
    if (ctx.loadedMaterialHandles.contains(matName))
        return WEngine::Nullable<WEngine::Material>(ctx.loadedMaterialHandles[matName]);
    return WEngine::Nullable<WEngine::Material>();
}

WEngine::Nullable<WEngine::Shader> Iris::GetShader(const std::string &shaderName)
{
    if (ctx.loadedShadersHandles.contains(shaderName))
        return WEngine::Nullable<WEngine::Shader>(ctx.loadedShadersHandles[shaderName]);
    return WEngine::Nullable<WEngine::Shader>();
}

WEngine::Nullable<WEngine::Shader> Iris::GetShader(WEngine::Material matQuery)
{
    if (ctx.loadedMaterials.size() < matQuery)
        return WEngine::Nullable<WEngine::Shader>();
    Vulkan_Material mat = ctx.loadedMaterials[matQuery - 1];
    return mat.materialShaderHandle;
}

WEngine::Nullable<WEngine::Material> Iris::ALLOC_CompileMaterial(const std::string& matName)
{
    auto check = GetMaterial(matName);

    if (check.HasValue())
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Material already {} compiled", matName));
        return WEngine::Nullable<WEngine::Material>();
    }


    WEngine::Material matHandle = CompileMaterial(ctx, matName);

    if (matHandle == 0)
        return WEngine::Nullable<WEngine::Material>();

    return WEngine::Nullable<WEngine::Material>(matHandle);
}

WEngine::Nullable<WEngine::Model> Iris::GetModel(const std::string &modelName)
{
    if (ctx.loadedModelHandles.contains(modelName))
        return WEngine::Nullable<WEngine::Model>(ctx.loadedModelHandles[modelName]);
    return WEngine::Nullable<WEngine::Model>();
}

WEngine::Nullable<WEngine::Model> Iris::ALLOC_CreateModel(const WEngine::ModelInfo &model)
{
    Vulkan_Model vkModel{};
    vkModel.vertexCount = model.vertices.size();
    vkModel.indexCount = model.indices.size();

    auto vertBuf = CreateVertexBuffer(ctx, stats, model.vertices);
    auto indBuf = CreateIndexBuffer(ctx, stats, model.indices);
    auto instBuf = InitInstanceBuffer(ctx, stats);

    // creation funcs already gave an error message
    if (vertBuf.first == VK_NULL_HANDLE)
        return WEngine::Nullable<WEngine::Model>();
    if (indBuf.first == VK_NULL_HANDLE)
        return WEngine::Nullable<WEngine::Model>();
    if (instBuf.first == VK_NULL_HANDLE)
        return WEngine::Nullable<WEngine::Model>();

    vkModel.vertexBuffer = vertBuf.first;
    vkModel.vertexAllocation = vertBuf.second;
    vkModel.indexBuffer = indBuf.first;
    vkModel.indexAllocation = indBuf.second;
    vkModel.instanceBuffer = instBuf.first;
    vkModel.instanceAllocation = instBuf.second;

    vkModel.instanceBufferSize = GPUSettingsVulkan::maxInstanceBufferSize;

    ctx.loadedModels.push_back(vkModel);
    WEngine::Model modelHandle = ctx.loadedModels.size();
    ctx.loadedModelHandles[model.name] = modelHandle;

    return WEngine::Nullable<WEngine::Model>(modelHandle);
}


void Iris::DRAWCALL_ClearFrame(WEngine::Color clearColor)
{
    VkClearColorValue clearCol{};
    WEngine::Colorf col(clearColor);
    clearCol = { col.red, col.green, col.blue, col.alpha };

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = clearCol;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkImageMemoryBarrier colBarrier{};
    colBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    colBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colBarrier.image = GetFbImage(ctx);
    colBarrier.srcAccessMask = 0;
    colBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    colBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colBarrier.subresourceRange.baseMipLevel = 0;
    colBarrier.subresourceRange.levelCount = 1;
    colBarrier.subresourceRange.baseArrayLayer = 0;
    colBarrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(GetFbCmdBuff(ctx), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &colBarrier);

    // never trusting vulkan with things passed in as reference!
    VkImageMemoryBarrier depthBarrier = colBarrier;
    depthBarrier.image = ctx.screen.depthImage;
    depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(GetFbCmdBuff(ctx), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &depthBarrier);

    VkRenderingAttachmentInfo colorAttachmentInfo{};
    colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachmentInfo.clearValue = clearValues[0];
    colorAttachmentInfo.imageView = GetFbImageView(ctx);
    colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingAttachmentInfo depthAttachmentInfo{};
    depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachmentInfo.clearValue = clearValues[1];
    depthAttachmentInfo.imageView = ctx.screen.depthImageView;
    depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = {0, 0};
    renderingInfo.renderArea.extent = {(uint32)GetFbResolution(ctx).x, (uint32)GetFbResolution(ctx).y}; // eye cancer
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachmentInfo;
    renderingInfo.pDepthAttachment = &depthAttachmentInfo;

    vkCmdBeginRendering(GetFbCmdBuff(ctx), &renderingInfo);
    ctx.currentBoundShader = 99999999;
}

void Iris::DRAWCALL_DrawModel(WEngine::Model model, WEngine::Material material, const WEngine::Mat4x4& mvp)
{
    if (!ctx.isCommandRecording)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Tried render model while no framebuffer was selected!");
        return;
    }

    Vulkan_Material vkMat = ctx.loadedMaterials[material - 1];
    Vulkan_Shader vkShader = ctx.loadedShaders[vkMat.materialShaderHandle - 1];
    if (ctx.currentBoundShader != vkMat.materialShaderHandle)
    {
        vkCmdBindPipeline(GetFbCmdBuff(ctx), VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipeline);
        ctx.currentBoundShader = vkMat.materialShaderHandle;
    }

    vkCmdBindDescriptorSets(GetFbCmdBuff(ctx), VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipelineLayout,
        0, 1, &ctx.lighting.descriptorSet, 0, nullptr);

    Vulkan_Model vkModel = ctx.loadedModels[model - 1];
    VkDeviceSize offset = 0;
    if (vkMat.hasTextures)
        vkCmdBindDescriptorSets(GetFbCmdBuff(ctx), VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipelineLayout,
            1, 1, &vkMat.materialDescriptorSet, 0, nullptr);
    vkCmdBindVertexBuffers(GetFbCmdBuff(ctx), 0, 1, &vkModel.vertexBuffer, &offset);
    vkCmdBindIndexBuffer(GetFbCmdBuff(ctx), vkModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    PopulatePushConstants(ctx, vkShader, mvp);

    vkCmdDrawIndexed(GetFbCmdBuff(ctx), vkModel.indexCount, 1, 0, 0, 0);

    stats.drawCallsThisFrame++;
}

void Iris::DRAWCALL_DrawModelInstanced(WEngine::Model model, WEngine::Material material,
    const WEngine::Mat4x4& vp, const wtl::vector<WEngine::InstanceData>& instanceMats)
{
    if (!ctx.isCommandRecording)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Tried render model while no framebuffer was selected!");
        return;
    }

    Vulkan_Material vkMat = ctx.loadedMaterials[material - 1];
    Vulkan_Shader vkShader = ctx.loadedShaders[vkMat.materialShaderHandle - 1];
    if (ctx.currentBoundShader != vkMat.materialShaderHandle)
    {
        vkCmdBindPipeline(GetFbCmdBuff(ctx), VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipeline);
        ctx.currentBoundShader = vkMat.materialShaderHandle;
    }

    Vulkan_Model& vkModel = ctx.loadedModels[model - 1];

    if (instanceMats.size() + vkModel.activeInstances > vkModel.instanceBufferSize)
        ExpandInstanceBuffer(ctx, stats, vkModel, instanceMats.size() + vkModel.activeInstances);

    WEngine::InstanceData* inst;
    vmaMapMemory(ctx.vcore.vmaAllocator, vkModel.instanceAllocation, (void**)&inst);

    memcpy(inst + vkModel.activeInstances, instanceMats.data(), instanceMats.size() * sizeof(WEngine::InstanceData));

    vmaUnmapMemory(ctx.vcore.vmaAllocator, vkModel.instanceAllocation);

    vkCmdBindDescriptorSets(GetFbCmdBuff(ctx), VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipelineLayout,
        0, 1, &ctx.lighting.descriptorSet, 0, nullptr);

    std::array<VkDeviceSize, 2> offsets{0, sizeof(WEngine::InstanceData) * vkModel.activeInstances};
    if (vkMat.hasTextures)
        vkCmdBindDescriptorSets(GetFbCmdBuff(ctx), VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipelineLayout,
            1, 1, &vkMat.materialDescriptorSet, 0, nullptr);
    vkCmdBindVertexBuffers(GetFbCmdBuff(ctx), 0, 1, &vkModel.vertexBuffer, &offsets[0]);
    vkCmdBindVertexBuffers(GetFbCmdBuff(ctx), 1, 1, &vkModel.instanceBuffer, &offsets[1]);
    vkCmdBindIndexBuffer(GetFbCmdBuff(ctx), vkModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    PopulatePushConstants(ctx, vkShader, vp);

    vkCmdDrawIndexed(GetFbCmdBuff(ctx), vkModel.indexCount, instanceMats.size(), 0, 0, 0);

    vkModel.activeInstances += instanceMats.size();
    stats.drawCallsThisFrame++;
}

void Iris::DRAWCALL_DrawModelInstancedStationary(WEngine::Model model, WEngine::Material material,
    const WEngine::Mat4x4& vp)
{
    if (!ctx.isCommandRecording)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Tried render model while no framebuffer was selected!");
        return;
    }

    auto alloc = ctx.statBuf.statBookkeep.FindNode(model, material);
    if (alloc.first == 0 && alloc.second == 0)
        return;

    Vulkan_Material& vkMat = ctx.loadedMaterials[material - 1];
    Vulkan_Shader& vkShader = ctx.loadedShaders[vkMat.materialShaderHandle - 1];
    if (ctx.currentBoundShader != vkMat.materialShaderHandle)
    {
        vkCmdBindPipeline(GetFbCmdBuff(ctx), VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipeline);
        ctx.currentBoundShader = vkMat.materialShaderHandle;
    }

    vkCmdBindDescriptorSets(GetFbCmdBuff(ctx), VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipelineLayout,
        0, 1, &ctx.lighting.descriptorSet, 0, nullptr);

    Vulkan_Model& vkModel = ctx.loadedModels[model - 1];

    uint64 count = alloc.second / sizeof(WEngine::InstanceData);

    std::array<VkDeviceSize, 2> offsets{0, alloc.first};
    if (vkMat.hasTextures)
        vkCmdBindDescriptorSets(GetFbCmdBuff(ctx), VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipelineLayout,
            1, 1, &vkMat.materialDescriptorSet, 0, nullptr);
    vkCmdBindVertexBuffers(GetFbCmdBuff(ctx), 0, 1, &vkModel.vertexBuffer, &offsets[0]);
    vkCmdBindVertexBuffers(GetFbCmdBuff(ctx), 1, 1, &ctx.statBuf.statBuffer, &offsets[1]);
    vkCmdBindIndexBuffer(GetFbCmdBuff(ctx), vkModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    PopulatePushConstants(ctx, vkShader, vp);

    vkCmdDrawIndexed(GetFbCmdBuff(ctx), vkModel.indexCount, count, 0, 0, 0);
    stats.drawCallsThisFrame++;
}

void Iris::DRAWCALL_ResetImGui()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void Iris::DRAWCALL_DrawImGui()
{
    if (!ctx.isCommandRecording)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Tried render ImGui while no framebuffer was selected!");
        return;
    }

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), GetFbCmdBuff(ctx));
}

void Iris::DRAWCALL_DrawToDisplay(SDL_Window *window)
{
    VkResult renderRes;
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &GetFbRenderFinishedSem(ctx);
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &ctx.screen.swapchain;
    presentInfo.pImageIndices = &ctx.screen.swapchainCurrentImage;
    presentInfo.pResults = &renderRes;

    vkQueuePresentKHR(ctx.queues.primaryDrawQueue, &presentInfo);

    ctx.screen.currentFrame = (ctx.screen.currentFrame + 1) % ctx.screen.swapchainImageCount;

    if (!ParseVkResult(renderRes))
        WEngine::WLog::ConsoleLog("Something went wrong during rendering!");

    stats.drawCallsLastFrame = stats.drawCallsThisFrame;
    stats.drawCallsThisFrame = 0;


    ctx.firstFrame = false;
}

WEngine::Nullable<WEngine::Framebuffer> Iris::ALLOC_CreateFramebuffer(const WEngine::Vector2 &resolution)
{
    Vulkan_RenderTarget target = CreateRenderTarget(ctx, stats, resolution);

    ctx.renderTargets.push_back(target);
    WEngine::Framebuffer handle = ctx.renderTargets.size();

    return handle;
}

void Iris::SETTING_SelectFramebufferForRender(const WEngine::Framebuffer framebuffer)
{
    if (ctx.renderTargets.size() < framebuffer)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Frame buffer handle out of scope!");
        return;
    }

    if (ctx.isCommandRecording)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Tried to change frame buffer in the middle of drawing!");
        return;
    }

    ctx.currentRenderTarget = &ctx.renderTargets[framebuffer - 1];

    vkResetFences(ctx.vcore.gpuDevice, 1, &GetFbEndOfFrameFence(ctx));
    vkResetCommandBuffer(GetFbCmdBuff(ctx), 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(GetFbCmdBuff(ctx), &beginInfo);

    ctx.isCommandRecording = true;
}

void Iris::SETTING_SelectFramebufferScreenForRender()
{
    if (ctx.isCommandRecording)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Tried to change frame buffer in the middle of drawing!");
        return;
    }

    ctx.currentRenderTarget = &ctx.displayTarget;

    vkResetFences(ctx.vcore.gpuDevice, 1, &GetFbEndOfFrameFence(ctx));
    vkResetCommandBuffer(GetFbCmdBuff(ctx), 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(GetFbCmdBuff(ctx), &beginInfo);

    ctx.isCommandRecording = true;
}

void Iris::SETTING_FinishFramebufferRender()
{
    bool isSwapchain = ctx.currentRenderTarget == &ctx.displayTarget;
    if (!ctx.isCommandRecording)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Tried to finish a framebuffer while none was selected!");
        return;
    }

    vkCmdEndRendering(GetFbCmdBuff(ctx));

    VkImageMemoryBarrier imgBarrier{};
    imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imgBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imgBarrier.image = GetFbImage(ctx);
    imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imgBarrier.dstAccessMask = 0;

    imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgBarrier.subresourceRange.baseMipLevel = 0;
    imgBarrier.subresourceRange.levelCount = 1;
    imgBarrier.subresourceRange.baseArrayLayer = 0;
    imgBarrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(GetFbCmdBuff(ctx), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imgBarrier);

    auto res = vkEndCommandBuffer(GetFbCmdBuff(ctx));
    if (!ParseVkResult(res))
        WEngine::WLog::ConsoleLog("Something went wrong after ending the command buffer!");

    ctx.isCommandRecording = false;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    if (isSwapchain)
    {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &GetFbImageAvailSem(ctx);
        submitInfo.pWaitDstStageMask = &waitStage;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &GetFbRenderFinishedSem(ctx);
    }
    else
    {
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.signalSemaphoreCount = 0;
    }

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &GetFbCmdBuff(ctx);

    vkQueueSubmit(ctx.queues.primaryDrawQueue, 1, &submitInfo, GetFbEndOfFrameFence(ctx));
}

uint64 Iris::GetVramUsage()
{
    return stats.vramUsage;
}

uint32 Iris::GetDrawCallCountLastFrame()
{
    return stats.drawCallsLastFrame;
}

bool Iris::IsFirstFrame()
{
    return ctx.firstFrame;
}

WEngine::Nullable<ImTextureID> Iris::FramebufferToImGui(WEngine::Framebuffer framebuffer)
{
    return WEngine::Nullable<ImTextureID>();
}

wtl::vector<MemListDebugInfo> Iris::GetStatInstBufAllocInfo()
{
    return ctx.statBuf.statBookkeep.GetDebugInfo();
}

void Iris::AddStationaryObjects(WEngine::Model model, WEngine::Material material,
    wtl::vector<WEngine::InstanceData> instanceMats)
{
    auto oldAlloc = ctx.statBuf.statBookkeep.FindNode(model, material);

    uint64 size = instanceMats.size() * sizeof(WEngine::InstanceData);
    auto newAlloc = ctx.statBuf.statBookkeep.InsertData(model, material, size);

    uint64 trueOffset = newAlloc.first / sizeof(WEngine::InstanceData);

    WEngine::InstanceData* data;

    vmaMapMemory(ctx.vcore.vmaAllocator, ctx.statBuf.statAllocation, (void**)&data);

    // case 1: new allocation
    if (oldAlloc.first == 0 && oldAlloc.second == 0)
    {
        memcpy(data + trueOffset, instanceMats.data(), size);
        vmaUnmapMemory(ctx.vcore.vmaAllocator, ctx.statBuf.statAllocation);
        return;
    }

    uint64 trueOldOffset = oldAlloc.first / sizeof(WEngine::InstanceData);
    uint64 trueOldSize = oldAlloc.second / sizeof(WEngine::InstanceData);

    // case 2: simple resize
    if (oldAlloc.first == newAlloc.first)
    {
        memcpy(data + trueOffset + trueOldSize, instanceMats.data(), size);
    }
    // case 3: reallocation
    else
    {
        memmove(data + trueOffset, data + trueOldOffset, oldAlloc.second);
        memcpy(data + trueOffset + trueOldSize, instanceMats.data(), size);
    }
    vmaUnmapMemory(ctx.vcore.vmaAllocator, ctx.statBuf.statAllocation);
}

void Iris::AssetIrisCommunication(WEngine::AssetIrisCommunication &mission)
{
    Vulkan_Texture tex;
    WEngine::Texture texHandle;
    switch (mission.commType)
    {
        case WEngine::AssetIrisCommunicationType::StoreTexture:
            WEngine::WLog::SetConsoleInfo();
            WEngine::WLog::ConsoleLog("Got store texture comms mission.");

            tex = CreateTexture(ctx, stats, mission.textureData);
            QueueTexture(ctx, tex, mission.textureData);

            ctx.loadedTextures.push_back(tex);
            texHandle = ctx.loadedTextures.size();
            mission.texReferenceOut = texHandle;

            break;
        case WEngine::AssetIrisCommunicationType::UnloadTexture:
            break;
    }
}
#endif
