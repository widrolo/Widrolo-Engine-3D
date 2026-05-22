#include "PerfTester.h"

#include "Engine/Types/SpawnArgs.h"


PerfTester::PerfTester(WEngine::Sector* testworld)
{
    WEngine::Transform t = WEngine::Transform();
    t.position = WEngine::Vector3::Zero;
    t.rotation = WEngine::Vector3::Zero;
    t.size = WEngine::Vector3::One;

    YAML::Node settings = YAML::Node();

    settings["meshName"] = "Cube";
    settings["instance"] = false;

    WEngine::ComponentArgs ca;
    ca.componentTypeId = 14;


    WEngine::SpawnArgs args;
    args.name = "e";
    args.transform = t;
    args.ca = {ca};

    const uint32 sideLen = 15;
    const float32 stride = 3.0f;
    uint32 counterModel = 0;
    uint32 counterShader = 0;
    for (uint32 i = 0; i < sideLen; ++i)
    {
        for (uint32 j = 0; j < sideLen; ++j)
        {
            for (uint32 k = 0; k < sideLen; ++k)
            {
                switch (counterShader)
                {
                    case 0: settings["shaderName"] = "triangleReg"; counterShader++; break;
                    case 1: settings["shaderName"] = "triangleInvert"; counterShader++; break;
                    case 2: settings["shaderName"] = "triangleBlue"; counterShader = 0; break;
                }

                if (counterModel % 2 == 0)
                {
                    settings["meshName"] = "Cube";
                    ca.componentRoot = settings;
                    args.ca = {ca};
                }
                else
                {
                    settings["meshName"] = "Monkey";
                    ca.componentRoot = settings;
                    args.ca = {ca};
                }
                args.transform = t;
                testworld->AddEntityPost(args);
                t.position.x += stride;
                counterModel++;
            }
            t.position.y -= stride;
            t.position.x = 0.0f;
        }
        t.position.z += stride;
        t.position.y = 0.0f;
    }


}
