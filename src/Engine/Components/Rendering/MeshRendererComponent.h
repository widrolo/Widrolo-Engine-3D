#pragma once
#include "Engine/Components/Component.h"
#include "Engine/Types/Rendering/GPU/Model.h"
#include "Engine/Types/Rendering/GPU/Shader.h"

namespace WEngine
{
    class MeshRendererComponent : public Component
    {
    public:
        MeshRendererComponent(Entity* e);

    public:
        void Awake(ComponentArgs ca) override;
        void Draw() override;

    private:
        Model m_model = 0;
        Shader m_shader = 0;
        bool m_isInstanceable = false;

        COMP_HASH(0xeb113cecf16966)

    };
}
