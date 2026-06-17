#include "VulkanShaders.h"

#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include <filesystem>

#include "VulkanHelpers.h"
#include "VulkanPipeline.h"
#include "Engine/Core/Handlers/AssetRepo.h"
#include "Engine/Core/System/Iris.h"
#include "Engine/Core/System/OS.h"
#include "Engine/Types/AssetMission.h"
#include "Engine/Types/CoreSystems.h"
#include "Engine/Types/Nullable.h"
#include "Engine/Types/Rendering/InstanceData.h"
#include "Engine/Types/Rendering/VertextData.h"
#include "Engine/Util/Log.h"

VkShaderModule CompileShader(const VulkanContext& ctx, const WEngine::SpirVAssetMission& spirvAssetMission)
{
    VkShaderModule module;
    VkShaderModuleCreateInfo shaderModuleInfo{};
    shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleInfo.codeSize = spirvAssetMission.shaderSize;
    shaderModuleInfo.pCode = spirvAssetMission.shaderCode;

    auto res = vkCreateShaderModule(ctx.vcore.gpuDevice, &shaderModuleInfo, ctx.vcore.allocator, &module);
    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create shader module!");
        return VK_NULL_HANDLE;
    }
    return module;
}

VkPipelineInputAssemblyStateCreateInfo CreatePipeline_InputAssembly()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    return inputAssemblyInfo;
}

VkPipelineVertexInputStateCreateInfo CreatePipeline_VertexDefinition()
{
    // it remains like this for now, but consider making this changeable pretty please
    const uint8 bindings = 2;
    const uint8 attributes = 8;

    auto bindDesc = wNewArr(VkVertexInputBindingDescription, bindings);
    bindDesc[0].binding = 0;
    bindDesc[0].stride = sizeof(WEngine::VertexData);
    bindDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindDesc[1].binding = 1;
    bindDesc[1].stride = sizeof(WEngine::InstanceData);
    bindDesc[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    // [0] = Position (Vector3)   |   [1] = Color (Vector3)   |   [2] = Color UV (Vector2)   |   [3] = Shadow UV (Vector2)
    // [4-7] = Model (Mat4x4)     |
    auto attributeDescriptions = wNewArr(VkVertexInputAttributeDescription, attributes);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Khronos had a meth party while making this one
    attributeDescriptions[0].offset = offsetof(WEngine::VertexData, position);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // Khronos had a meth party while making this one
    attributeDescriptions[1].offset = offsetof(WEngine::VertexData, vertColor);
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT; // Khronos had a meth party while making this one
    attributeDescriptions[2].offset = offsetof(WEngine::VertexData, uv0Coord);
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT; // Khronos had a meth party while making this one
    attributeDescriptions[3].offset = offsetof(WEngine::VertexData, uv1Coord);

    attributeDescriptions[4] = {4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 0};
    attributeDescriptions[5] = {5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4};
    attributeDescriptions[6] = {6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8};
    attributeDescriptions[7] = {7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = bindings;
    vertexInputInfo.pVertexBindingDescriptions = bindDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = attributes;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    return vertexInputInfo;
}

VkPipelineShaderStageCreateInfo CreatePipeline_ShaderStange_Vertex(const VulkanContext& ctx, const std::string& shaderName)
{
    // Sperm Vee
    WEngine::SpirVAssetMission vertexShaderCode{};
    vertexShaderCode.shaderType = WEngine::SpirVAssetMission::VertexShader;
    vertexShaderCode.name = shaderName;
    WEngine::CoreSystems::GetAssetRepo()->GetAsset(vertexShaderCode);

    VkPipelineShaderStageCreateInfo shaderStage{};

    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStage.pName = "main";
    shaderStage.module = CompileShader(ctx, vertexShaderCode);
    wFree(vertexShaderCode.shaderCode);

    return shaderStage;
}

VkPipelineShaderStageCreateInfo CreatePipeline_ShaderStange_Fragment(const VulkanContext& ctx, const std::string& shaderName)
{
    WEngine::SpirVAssetMission fragmentShaderCode{};
    fragmentShaderCode.shaderType = WEngine::SpirVAssetMission::FragmentShader;
    fragmentShaderCode.name = shaderName;
    WEngine::CoreSystems::GetAssetRepo()->GetAsset(fragmentShaderCode);

    VkPipelineShaderStageCreateInfo shaderStage{};

    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStage.pName = "main";
    shaderStage.module = CompileShader(ctx, fragmentShaderCode);
    wFree(fragmentShaderCode.shaderCode);

    return shaderStage;
}

VkPipeline CreatePipeline(VulkanContext& ctx, VkRenderPass renderPass, const WEngine::ShaderDefinition& shaderDef,
    VkPipelineLayout pipelineLayout)
{
    VkPipelineRasterizationStateCreateInfo rasterInfo{};
    rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterInfo.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState blendState{};
    blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo blendInfo{};
    blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendInfo.attachmentCount = 1;
    blendInfo.pAttachments = &blendState;

    VkPipelineViewportStateCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewInfo.viewportCount = 1;
    viewInfo.scissorCount = 1;

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::array<VkDynamicState, 2> dynamics{};
    dynamics[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamics[1] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDynamicStateCreateInfo dynamicInfo{};
    dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo.dynamicStateCount = dynamics.size();
    dynamicInfo.pDynamicStates = dynamics.data();

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
    shaderStages[0] = CreatePipeline_ShaderStange_Vertex(ctx, shaderDef.vertexCodeName);
    shaderStages[1] = CreatePipeline_ShaderStange_Fragment(ctx, shaderDef.fragmentCodeName);

    auto inputAssembly = CreatePipeline_InputAssembly();
    auto vertexDefinition = CreatePipeline_VertexDefinition();

    VkGraphicsPipelineCreateInfo pipeInfo{};
    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeInfo.pInputAssemblyState = &inputAssembly;
    pipeInfo.pVertexInputState = &vertexDefinition;
    pipeInfo.pRasterizationState = &rasterInfo;
    pipeInfo.pColorBlendState = &blendInfo;
    pipeInfo.pViewportState = &viewInfo;
    pipeInfo.pDepthStencilState = &depthStencilInfo;
    pipeInfo.pMultisampleState = &multisampleInfo;
    pipeInfo.pDynamicState = &dynamicInfo;
    pipeInfo.stageCount = shaderStages.size();
    pipeInfo.pStages = shaderStages.data();
    pipeInfo.renderPass = renderPass;
    pipeInfo.layout = pipelineLayout;

    VkPipeline pipeline;
    auto res = vkCreateGraphicsPipelines(ctx.vcore.gpuDevice, VK_NULL_HANDLE, 1, &pipeInfo, ctx.vcore.allocator, &pipeline);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to create graphics pipeline.");
        return VK_NULL_HANDLE;
    }

    vkDestroyShaderModule(ctx.vcore.gpuDevice, shaderStages[0].module, ctx.vcore.allocator);
    vkDestroyShaderModule(ctx.vcore.gpuDevice, shaderStages[1].module, ctx.vcore.allocator);

    wFree((void*)vertexDefinition.pVertexBindingDescriptions);
    wFree((void*)vertexDefinition.pVertexAttributeDescriptions);

    return pipeline;
}

void SaturateDescriptorSet(VulkanContext& ctx, Vulkan_Material& material)
{
    auto& set = material.materialDescriptorSet;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView   = material.albedo;
    imageInfo.sampler     = material.albedoSampler;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(ctx.vcore.gpuDevice, 1, &write, 0, nullptr);
}


WEngine::MaterialDefinition ParseMaterialDefinition(VulkanContext &ctx, const YAML::Node &root);

void TryCompileAllShaders(VulkanContext &ctx)
{
    std::string dir = WEngine::CoreSystems::GetAssetRepo()->GetDataPath() + EngineSettings::shaderPath + "definitions";
    auto files = OS::GetAllFileNamesInDir(dir);

    for (auto& file : files)
    {
        std::filesystem::path p(file);
        file = p.stem().string();
    }

    for (const auto& file : files)
    {
        if (file == "$Sample")
            continue;

        WEngine::YamlAssetMission mission;
        mission.name = "../" + EngineSettings::shaderPath + "definitions/" + file;
        WEngine::CoreSystems::GetAssetRepo()->GetAsset(mission);

        WEngine::ShaderDefinition def{};
        def.Parse(mission.root);
        if (!def.valid)
            continue;

        Vulkan_Shader shader{};

        auto descPool = CreateDescriptorPool(ctx, def);
        auto descLayout = CreateDescriptorSetLayout(ctx, def);
        auto pipeLayout = CreatePipelineLayout(ctx, descLayout);
        auto pipeline = CreatePipeline(ctx, ctx.renderPass, def, pipeLayout);

        shader.descriptorPool = descPool;
        shader.descriptorSetLayout = descLayout;
        shader.pipelineLayout = pipeLayout;
        shader.pipeline = pipeline;
        shader.shaderDefinition = def;

        ctx.loadedShaders.push_back(shader);
        WEngine::Shader shaderHandle = ctx.loadedShaders.size();
        ctx.loadedShadersHandles[def.name] = shaderHandle;

        WEngine::WLog::SetConsoleSuccess();
        WEngine::WLog::ConsoleLog(std::format("Shader \"{}\" successfully compiled!", def.name));
    }
}

WEngine::Material CompileMaterial(VulkanContext &ctx, const std::string &matName)
{
    WEngine::YamlAssetMission mission;
    mission.name = "../Materials/" + matName;
    WEngine::CoreSystems::GetAssetRepo()->GetAsset(mission);

    WEngine::MaterialDefinition matDef;
    matDef.Parse(mission.root);

    if (!matDef.valid)
        return 0;

    // lets hope iris never calls this function through GetShader
    auto shaderN = Iris::GetShader(matDef.shaderName);

    if (!shaderN.HasValue())
        return 0;

    Vulkan_Shader& shader = ctx.loadedShaders[shaderN.GetValue() - 1];
    Vulkan_Material mat;
    mat.materialShaderHandle = shaderN.GetValue();

    if (!matDef.texturesPackaging.empty())
    {
        mat.materialDescriptorSet = CreateDescriptorSet(ctx, shader);
        mat.hasTextures = true;
    }
    ctx.loadedMaterials.push_back(mat);
    WEngine::Material matHandle = ctx.loadedMaterials.size();
    ctx.loadedMaterialHandles[matDef.name] = matHandle;

    WEngine::WLog::SetConsoleSuccess();
    WEngine::WLog::ConsoleLog(std::format("Material \"{}\" compiled", matName));

    return matHandle;
}

#endif
