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

    if (!SetupImGuiDescriptorPool(ctx))
        return false;

    if (!SetupCommandPool(ctx))
        return false;

    if (!SetupStationaryInstanceBuffer(ctx, stats))
        return false;


    ctx.cmdBufs.resize(ctx.screen.swapchainImages.size());
    for (uint32 i = 0; i < ctx.screen.swapchainImages.size(); i++)
        ctx.cmdBufs[i] = CreateCommandBuffer(ctx, ctx.commandPool);
    ctx.renderPass = CreateBasicRenderPass(ctx);

    SetupSwapchainFramebuffers(ctx, stats);

    TryCompileAllShaders(ctx);

    return true;
}

void Iris::SETTING_ConfigureImGui(SDL_Window *window)
{
    ImGui_ImplSDL3_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = VK_API_VERSION_1_4;
    initInfo.Instance = ctx.vcore.instance;
    initInfo.PhysicalDevice = ctx.vcore.gpuPhysicalDevice;
    initInfo.Device = ctx.vcore.gpuDevice;
    initInfo.Allocator = ctx.vcore.allocator;
    initInfo.QueueFamily = ctx.queues.primaryDrawQueueFamilyIndex;
    initInfo.Queue = ctx.queues.primaryDrawQueue;
    initInfo.ImageCount = ctx.screen.swapchainImages.size();
    initInfo.MinImageCount = ctx.screen.swapchainImages.size();
    initInfo.DescriptorPoolSize = 8;
    initInfo.PipelineInfoMain.RenderPass = ctx.renderPass;
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);
}


void Iris::SETTING_BeginNewFrame()
{
    vkWaitForFences(ctx.vcore.gpuDevice, 1, &ctx.screen.endOfFrameFences[ctx.screen.currentFrame], VK_TRUE, UINT64_MAX);

    for (auto& buf : ctx.bufferGraveyard[ctx.screen.currentFrame])
        vmaDestroyBuffer(ctx.vcore.vmaAllocator, buf.first, buf.second);
    ctx.bufferGraveyard[ctx.screen.currentFrame].clear();

    vkResetFences(ctx.vcore.gpuDevice, 1, &ctx.screen.endOfFrameFences[ctx.screen.currentFrame]);
    vkAcquireNextImageKHR(ctx.vcore.gpuDevice, ctx.screen.swapchain, max_uint64,
        ctx.screen.imageAvailableSems[ctx.screen.currentFrame], VK_NULL_HANDLE, &ctx.screen.swapchainCurrentImage);


    vkResetCommandBuffer(ctx.cmdBufs[ctx.screen.currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(ctx.cmdBufs[ctx.screen.currentFrame], &beginInfo);

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
    vkCmdSetViewport(ctx.cmdBufs[ctx.screen.currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent.width = size.x;
    scissor.extent.height = size.y;
    vkCmdSetScissor(ctx.cmdBufs[ctx.screen.currentFrame], 0, 1, &scissor);
}

WEngine::Nullable<WEngine::Material> Iris::GetMaterial(const std::string &matName)
{
    if (ctx.loadedMaterialHandles.contains(matName))
        return WEngine::Nullable<WEngine::Material>(ctx.loadedMaterialHandles[matName]);
    return WEngine::Nullable<WEngine::Material>();
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

    //auto pipeline = CreatePipeline(ctx, ctx.renderPass, matName);
//
    //ctx.loadedShaders.push_back({pipeline});
    //WEngine::Shader shaderHandle = ctx.loadedShaders.size();
    //ctx.loadedShadersHandles[matName] = shaderHandle;
//
    //return WEngine::Nullable<WEngine::Material>(shaderHandle);
}

WEngine::Nullable<WEngine::Model> Iris::GetModel(const std::string &modelName)
{
    if (ctx.loadedModelHandles.contains(modelName))
        return WEngine::Nullable<WEngine::Shader>(ctx.loadedModelHandles[modelName]);
    return WEngine::Nullable<WEngine::Shader>();
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

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = ctx.renderPass;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {(uint32)EngineSettings::resolution.x, (uint32)EngineSettings::resolution.y};
    renderPassInfo.framebuffer = ctx.screen.swapchainFramebuffers[ctx.screen.swapchainCurrentImage];
    ctx.currentBoundShader = 99999999;
    vkCmdBeginRenderPass(ctx.cmdBufs[ctx.screen.currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Iris::DRAWCALL_DrawModel(WEngine::Model model, WEngine::Shader shader, const WEngine::ShaderSettings &settings)
{
    Vulkan_Shader vkShader;
    if (ctx.currentBoundShader != shader)
    {
        vkShader = ctx.loadedShaders[shader - 1];
        vkCmdBindPipeline(ctx.cmdBufs[ctx.screen.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipeline);
        ctx.currentBoundShader = shader;
    }

    WEngine::Mat4x4 mvp = std::get<WEngine::Mat4x4>(settings[0].option);
    auto mvpRaw = mvp.GetRawData();

    Vulkan_Model vkModel = ctx.loadedModels[model - 1];
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(ctx.cmdBufs[ctx.screen.currentFrame], 0, 1, &vkModel.vertexBuffer, &offset);
    vkCmdBindIndexBuffer(ctx.cmdBufs[ctx.screen.currentFrame], vkModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdPushConstants(ctx.cmdBufs[ctx.screen.currentFrame], vkShader.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
        sizeof(WEngine::Mat4x4), &mvpRaw);

    vkCmdDrawIndexed(ctx.cmdBufs[ctx.screen.currentFrame], vkModel.indexCount, 1, 0, 0, 0);

    stats.drawCallsThisFrame++;
}

void Iris::DRAWCALL_DrawModelInstanced(WEngine::Model model, WEngine::Shader shader,
    const WEngine::ShaderSettings &settings, const wtl::vector<WEngine::InstanceData>& instanceMats)
{
    Vulkan_Shader vkShader;
    if (ctx.currentBoundShader != shader)
    {
        vkShader = ctx.loadedShaders[shader - 1];
        vkCmdBindPipeline(ctx.cmdBufs[ctx.screen.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipeline);
        ctx.currentBoundShader = shader;
    }


    WEngine::Mat4x4 vp = std::get<WEngine::Mat4x4>(settings[0].option);
    auto vpRaw = vp.GetRawData();

    Vulkan_Model& vkModel = ctx.loadedModels[model - 1];

    if (instanceMats.size() + vkModel.activeInstances > vkModel.instanceBufferSize)
        ExpandInstanceBuffer(ctx, stats, vkModel, instanceMats.size() + vkModel.activeInstances);

    WEngine::InstanceData* inst;
    vmaMapMemory(ctx.vcore.vmaAllocator, vkModel.instanceAllocation, (void**)&inst);

    memcpy(inst + vkModel.activeInstances, instanceMats.data(), instanceMats.size() * sizeof(WEngine::InstanceData));

    vmaUnmapMemory(ctx.vcore.vmaAllocator, vkModel.instanceAllocation);

    std::array<VkDeviceSize, 2> offsets{0, sizeof(WEngine::InstanceData) * vkModel.activeInstances};
    vkCmdBindVertexBuffers(ctx.cmdBufs[ctx.screen.currentFrame], 0, 1, &vkModel.vertexBuffer, &offsets[0]);
    vkCmdBindVertexBuffers(ctx.cmdBufs[ctx.screen.currentFrame], 1, 1, &vkModel.instanceBuffer, &offsets[1]);
    vkCmdBindIndexBuffer(ctx.cmdBufs[ctx.screen.currentFrame], vkModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdPushConstants(ctx.cmdBufs[ctx.screen.currentFrame], vkShader.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
        sizeof(WEngine::Mat4x4), &vpRaw);

    vkCmdDrawIndexed(ctx.cmdBufs[ctx.screen.currentFrame], vkModel.indexCount, instanceMats.size(), 0, 0, 0);

    vkModel.activeInstances += instanceMats.size();
    stats.drawCallsThisFrame++;
}

void Iris::DRAWCALL_DrawModelInstancedStationary(WEngine::Model model, WEngine::Shader shader,
    const WEngine::ShaderSettings &settings)
{
    auto alloc = ctx.statBuf.statBookkeep.FindNode(model, shader);
    if (alloc.first == 0 && alloc.second == 0)
        return;

    Vulkan_Shader vkShader;
    if (ctx.currentBoundShader != shader)
    {
        vkShader = ctx.loadedShaders[shader - 1];
        vkCmdBindPipeline(ctx.cmdBufs[ctx.screen.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader.pipeline);
        ctx.currentBoundShader = shader;
    }

    Vulkan_Model& vkModel = ctx.loadedModels[model - 1];
    WEngine::Mat4x4 vp = std::get<WEngine::Mat4x4>(settings[0].option);
    auto vpRaw = vp.GetRawData();

    uint64 offset = alloc.first / sizeof(WEngine::InstanceData);
    uint64 count = alloc.second / sizeof(WEngine::InstanceData);

    std::array<VkDeviceSize, 2> offsets{0, alloc.first};
    vkCmdBindVertexBuffers(ctx.cmdBufs[ctx.screen.currentFrame], 0, 1, &vkModel.vertexBuffer, &offsets[0]);
    vkCmdBindVertexBuffers(ctx.cmdBufs[ctx.screen.currentFrame], 1, 1, &ctx.statBuf.statBuffer, &offsets[1]);
    vkCmdBindIndexBuffer(ctx.cmdBufs[ctx.screen.currentFrame], vkModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdPushConstants(ctx.cmdBufs[ctx.screen.currentFrame], vkShader.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
        sizeof(WEngine::Mat4x4), &vpRaw);

    vkCmdDrawIndexed(ctx.cmdBufs[ctx.screen.currentFrame], vkModel.indexCount, count, 0, 0, 0);
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
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), ctx.cmdBufs[ctx.screen.currentFrame]);
}

void Iris::DRAWCALL_SwapBuffers(SDL_Window *window)
{
    vkCmdEndRenderPass(ctx.cmdBufs[ctx.screen.currentFrame]);
    auto res = vkEndCommandBuffer(ctx.cmdBufs[ctx.screen.currentFrame]);
    if (!ParseVkResult(res))
        WEngine::WLog::ConsoleLog("Something went wrong after ending the command buffer!");

    // just for now
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &ctx.screen.imageAvailableSems[ctx.screen.currentFrame];
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &ctx.screen.renderFinishedSems[ctx.screen.currentFrame];
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &ctx.cmdBufs[ctx.screen.currentFrame];

    vkQueueSubmit(ctx.queues.primaryDrawQueue, 1, &submitInfo, ctx.screen.endOfFrameFences[ctx.screen.currentFrame]);

    VkResult renderRes;
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &ctx.screen.renderFinishedSems[ctx.screen.currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &ctx.screen.swapchain;
    presentInfo.pImageIndices = &ctx.screen.swapchainCurrentImage;
    presentInfo.pResults = &renderRes;

    vkQueuePresentKHR(ctx.queues.primaryDrawQueue, &presentInfo);

    ctx.screen.currentFrame = (ctx.screen.currentFrame + 1) % ctx.screen.swapchainImages.size();

    if (!ParseVkResult(renderRes))
        WEngine::WLog::ConsoleLog("Something went wrong during rendering!");

    stats.drawCallsLastFrame = stats.drawCallsThisFrame;
    stats.drawCallsThisFrame = 0;
}

uint64 Iris::GetVramUsage()
{
    return stats.vramUsage;
}

uint32 Iris::GetDrawCallCountLastFrame()
{
    return stats.drawCallsLastFrame;
}

WEngine::Nullable<ImTextureID> Iris::FramebufferToImGui(WEngine::Framebuffer framebuffer)
{
    return WEngine::Nullable<ImTextureID>();
}

wtl::vector<MemListDebugInfo> Iris::GetStatInstBufAllocInfo()
{
    return ctx.statBuf.statBookkeep.GetDebugInfo();
}

void Iris::AddStationaryObjects(WEngine::Model model, WEngine::Shader shader,
    wtl::vector<WEngine::InstanceData> instanceMats)
{
    auto oldAlloc = ctx.statBuf.statBookkeep.FindNode(model, shader);

    uint64 size = instanceMats.size() * sizeof(WEngine::InstanceData);
    auto newAlloc = ctx.statBuf.statBookkeep.InsertData(model, shader, size);

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

#endif
