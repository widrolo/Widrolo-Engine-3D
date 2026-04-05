#include "AnimationEditorWidget.h"

#include <cstring>
#include <fstream>

#include "Engine/Util/Log.h"
#include <yaml-cpp/yaml.h>

#include "Editor/Types/EditorSystems.h"
#include "Engine/EngineDefines.h"
#include "Engine/Core/Handlers/AssetRepo.h"
#include "Engine/imgui/imgui_internal.h"

using namespace WEditor;

void AnimationEditorWidget::Setup()
{
    m_widgetName = "Animation Editor";
    m_windowFlags |= ImGuiWindowFlags_NoDocking;
    m_animName[0] = '\0';
    AnimationEditorState::currentAnimationInfo.m_animationName = "";
    AnimationEditorState::selectedAnimation = nullptr;
    AnimationEditorState::selectedSubtype = nullptr;
    AnimationEditorState::selectedArchetype = nullptr;
    AnimationEditorState::causeForSavePresent = false;
}

void AnimationEditorWidget::RenderInternal()
{
    SetSize(WEngine::Vector2(700, 500));

    AnimTopBar();
    AnimTree();
    ImGui::SameLine();
    AnimEditor();
}

void AnimationEditorWidget::AnimTopBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20.f, 10.f));
    ImGui::InputText("##", m_animName, 128, ImGuiInputTextFlags_ElideLeft);
    ImGui::SameLine();
    if (ImGui::Button("Load"))
        LoadAnimation(m_animName);
    ImGui::SameLine();

    std::string saveString = "Save";
    if (AnimationEditorState::causeForSavePresent)
        saveString += "*";

    if (ImGui::Button(saveString.c_str()))
        SaveSelectedAnimationTree();
    ImGui::PopStyleVar();

    ImGui::Separator();

    auto& info = AnimationEditorState::currentAnimationInfo;
    if (info.m_animationName.empty())
        return;

    int32 framerate = info.m_framerate;
    bool changedFPS = ImGui::InputInt("Framerate", &framerate);
    info.m_framerate = framerate;

    ImGui::SameLine();

    if (ImGui::Button("Add New Archetype"))
    {
        auto newArch = WEngine::AnimationInformation::AnimationArchetype();
        newArch.name = "New Archetype";
        info.m_archetypes.push_back(newArch);
        AnimationEditorState::causeForSavePresent = true;
    }

    if (changedFPS)
        AnimationEditorState::causeForSavePresent = true;

    ImGui::Separator();
}

void AnimationEditorWidget::AnimTree()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;

    ImGui::BeginChild("anim types", ImVec2(ImGui::GetContentRegionAvail().x * 0.3f, ImGui::GetContentRegionAvail().y), 0, window_flags);
    ImGui::Text("Animation Tree");

    ShowAnimationTree();

    ImGui::EndChild();
}

void AnimationEditorWidget::AnimEditor()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::BeginChild("anim editor", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), 0, window_flags);
    ImGui::Text("Animation Editor");

    if (AnimationEditorState::selectedAnimation != nullptr)
        ShowAnimationEditor();
    else if (AnimationEditorState::selectedSubtype != nullptr)
        ShowSubtypeEditor();
    else if (AnimationEditorState::selectedArchetype != nullptr)
        ShowArchetypeEditor();
    else
        ImGui::Text("No animation selected");

    ImGui::EndChild();
}

void AnimationEditorWidget::ShowAnimationTree()
{
    auto& anim = AnimationEditorState::currentAnimationInfo;
    std::string& animName = AnimationEditorState::currentAnimationInfo.m_animationName;
    if (animName.empty())
    {
        ImGui::Text("No animation selected");
        return;
    }

    bool selAnim = false;
    bool selSub = false;

    // Pyramid of Giza
    if (ImGui::TreeNode(animName.c_str()))
    {
        for (auto& archetype : anim.m_archetypes)
        {
            ImGui::PushID(&archetype);
            if (ImGui::TreeNode(archetype.name.c_str()))
            {
                for (auto& subtype : archetype.subtypes)
                {
                    ImGui::PushID(&subtype);
                    if (ImGui::TreeNode(subtype.name.c_str()))
                    {
                        for (auto& animation : subtype.animations)
                        {
                            ImGui::PushID(&animation);
                            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                            ImGui::TreeNodeEx(animation.name.c_str(), nodeFlags);
                            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                            {
                                AnimationEditorState::selectedAnimation = &animation;
                                AnimationEditorState::selectedArchetype = nullptr;
                                AnimationEditorState::selectedSubtype = nullptr;
                                selAnim = true;
                            }
                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() && !selAnim)
                    {
                        AnimationEditorState::selectedAnimation = nullptr;
                        AnimationEditorState::selectedArchetype = nullptr;
                        AnimationEditorState::selectedSubtype = &subtype;
                        selSub = true;
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() && !selSub && !selAnim)
            {
                AnimationEditorState::selectedAnimation = nullptr;
                AnimationEditorState::selectedArchetype = &archetype;
                AnimationEditorState::selectedSubtype = nullptr;
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }
}

void AnimationEditorWidget::ShowAnimationEditor()
{
    ImGui::SeparatorText("Animation");
    auto anim = AnimationEditorState::selectedAnimation;
    static WEngine::Animation* cachedAnim = nullptr;

    static char animName[128];

    if (cachedAnim != anim)
    {
        std::fill_n(animName, 128, 0);
        std::strncpy(animName, anim->name.c_str(), anim->name.length());
        cachedAnim = anim;
    }


    ImGui::InputText("##", animName, 128, ImGuiInputTextFlags_ElideLeft);
    ImGui::SameLine();

    if (ImGui::Button("Apply"))
    {
        anim->name = animName;
        AnimationEditorState::causeForSavePresent = true;
    }

    int32 frames = anim->frameCount;
    bool changedRange = ImGui::InputInt("Frames of Animation", &frames);
    anim->frameCount = frames;

    if (changedRange)
        AnimationEditorState::causeForSavePresent = true;
}

void AnimationEditorWidget::ShowSubtypeEditor()
{
    ImGui::SeparatorText("Subtype");
    auto subt = AnimationEditorState::selectedSubtype;
    static WEngine::AnimationInformation::AnimationSubtype* cachedSubt = nullptr;

    static char animName[128];

    if (cachedSubt != subt)
    {
        std::fill_n(animName, 128, 0);
        std::strncpy(animName, subt->name.c_str(), subt->name.length() + 1);
        cachedSubt = subt;
    }

    ImGui::InputText("##", animName, 128, ImGuiInputTextFlags_ElideLeft);
    ImGui::SameLine();

    if (ImGui::Button("Apply"))
    {
        subt->name = animName;
        AnimationEditorState::causeForSavePresent = true;
    }
    if (ImGui::Button("Add New Animation"))
    {
        auto newAnim = WEngine::Animation();
        newAnim.name = "New Animation";
        subt->animations.push_back(newAnim);
        AnimationEditorState::causeForSavePresent = true;
    }
}

void AnimationEditorWidget::ShowArchetypeEditor()
{
    ImGui::SeparatorText("Archetype");
    auto arch = AnimationEditorState::selectedArchetype;
    static WEngine::AnimationInformation::AnimationArchetype* cachedArch = nullptr;

    static char animName[128];

    if (cachedArch != arch)
    {
        std::fill_n(animName, 128, 0);
        std::strncpy(animName, arch->name.c_str(), arch->name.length() + 1);
        cachedArch = arch;
    }

    ImGui::InputText("##", animName, 128, ImGuiInputTextFlags_ElideLeft);
    ImGui::SameLine();

    if (ImGui::Button("Apply"))
    {
        arch->name = animName;
        AnimationEditorState::causeForSavePresent = true;
    }
    if (ImGui::Button("Add New Subtype"))
    {
        auto newSubt = WEngine::AnimationInformation::AnimationSubtype();
        newSubt.name = "New Subtype";
        arch->subtypes.push_back(newSubt);
        AnimationEditorState::causeForSavePresent = true;
    }
}

void AnimationEditorWidget::LoadAnimation(const std::string& animName)
{
    AnimationEditorState::selectedAnimation = nullptr;
    AnimationEditorState::selectedArchetype = nullptr;
    AnimationEditorState::selectedSubtype = nullptr;
    AnimationEditorState::currentAnimationInfo = WEngine::AnimationParser::ParseAnimation(animName);
}

void AnimationEditorWidget::SaveSelectedAnimationTree()
{
    const auto& info = AnimationEditorState::currentAnimationInfo;
    if (info.m_animationName.empty())
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("No animation selected to save!");
        return;
    }

    YAML::Node root;
    YAML::Node cutouts;
    root["atlas"] = cutouts;


    YAML::Node animInfo;
    root["animation-info"] = animInfo;
    animInfo["framerate"] = (uint32)info.m_framerate;
    YAML::Node archs;
    animInfo["archetypes"] = archs;

    uint32 animIndex = 0;
    uint16 frameIndex = 0;

    for (const auto& arch : info.m_archetypes)
    {
        YAML::Node archNode;
        archs[arch.name] = archNode;
        archNode["name"] = arch.name;
        for (const auto& subt : arch.subtypes)
        {
            YAML::Node subNode;
            archNode[subt.name] = subNode;
            subNode["name"] = subt.name;
            for (const auto& anim : subt.animations)
            {
                YAML::Node animNode;
                subNode[anim.name] = animNode;
                animNode["name"] = anim.name;
                YAML::Node range;

                if (anim.frameCount == 0)
                {
                    range.push_back(0);
                    range.push_back(0);
                }
                else
                {
                    range.push_back(frameIndex);
                    frameIndex += anim.frameCount;
                    range.push_back(frameIndex - 1);
                }

                range.SetStyle(YAML::EmitterStyle::Flow);
                animNode["range"] = range;
                animIndex++;
            }
        }
    }

    uint16 cutFrames = std::ceil(std::sqrt(frameIndex));
    cutouts["h_frames"] = cutFrames;
    cutouts["v_frames"] = cutFrames;

    YAML::Emitter out;
    out << root;

    std::string animName = info.m_animationName;
    std::transform(animName.begin(), animName.end(), animName.begin(), [](unsigned char c) { return std::tolower(c); });

    std::string path = EditorSystems::GetAssetRepo()->GetDataPath();
    path += EngineSettings::spritePath;
    path += animName;
    path += ".yaml";

    std::ofstream fout(path);
    fout << out.c_str();
    fout.close();

    WEngine::WLog::ConsoleLog(std::format("Saved changes to:\n\t\t{}", path));
    AnimationEditorState::causeForSavePresent = false;
}
