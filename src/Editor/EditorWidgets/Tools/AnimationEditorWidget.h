#pragma once
#include "Engine/Core/Widget.h"
#include "Engine/Types/Rendering/Animation.h"

namespace WEditor
{
    struct AnimationEditorState
    {
        _GLOBAL_ WEngine::AnimationInformation currentAnimationInfo;
        _GLOBAL_ WEngine::AnimationInformation::AnimationArchetype* selectedArchetype;
        _GLOBAL_ WEngine::AnimationInformation::AnimationSubtype* selectedSubtype;
        _GLOBAL_ WEngine::Animation* selectedAnimation;
        _GLOBAL_ bool causeForSavePresent;
    };

    class AnimationEditorWidget : public WEngine::Widget
    {
    public:
        using Widget::Widget;
    public:
        void Setup() override;
    protected:
        void RenderInternal() override;

    private:
        char m_animName[128];

        void AnimTopBar();
        void AnimTree();
        void AnimEditor();
        void ShowAnimationTree();
        void ShowAnimationEditor();
        void ShowSubtypeEditor();
        void ShowArchetypeEditor();

        void LoadAnimation(const std::string& animName);
        void SaveSelectedAnimationTree();
    };
}

