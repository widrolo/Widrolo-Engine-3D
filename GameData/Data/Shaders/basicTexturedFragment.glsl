#version 450

layout(location = 1) in vec2 inUV0;
layout(location = 3) in vec3 inNormal;

layout(set = 1, binding = 0) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(tex, inUV0);
}