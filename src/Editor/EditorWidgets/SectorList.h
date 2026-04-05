#pragma once

#include <Engine/Core/Widget.h>
#include <yaml-cpp/yaml.h>

namespace WEngine
{
    class Sector;
    class Entity;
    class Component;
}

namespace WEditor
{
    class SectorList : public WEngine::Widget
    {
    public:
        using Widget::Widget;

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;

    private:
        char m_sectorName[128] = ""; // i really hate this!

    private:
        void ShowSectorList();
        void LoadSector();
        void SaveSectorToYaml(WEngine::Sector* sector);
        YAML::Node GetEntityInfoYaml(WEngine::Entity* e);
        YAML::Node GetComponentInfoYaml(WEngine::Component* comp);
    };
}

