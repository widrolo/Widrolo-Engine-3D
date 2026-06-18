#version 450

layout(location = 1) in vec2 inUV0;

layout(binding = 0) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;

void main()
{
    vec2 realUV = inUV0;
    realUV.y = 1 - realUV.y;
    outColor = texture(tex, realUV);
}