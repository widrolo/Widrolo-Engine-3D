#include "BlackboardVariable.h"

using namespace WEngine;

std::string BlackboardVariableType_Names[(int)BlackboardVariableType::BlackboardVariableType_Count]
{
    "Boolean",
    "Byte",
    "Number",
    "LargeNumber",
    "Float",
    "String",
    "Vector2",
    "Vector3",
    "Color",
    "Entity"
};