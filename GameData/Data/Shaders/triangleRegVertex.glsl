#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 uv0;
layout(location = 3) in vec2 uv1;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
} pc;

layout(location = 0) out vec3 outColor;

void main()
{
    outColor = inColor;
    //outColor = vec3(1.0, 1.0, 1.0);
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
}