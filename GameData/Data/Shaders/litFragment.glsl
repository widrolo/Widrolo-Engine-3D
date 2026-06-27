#version 450

layout(location = 1) in vec2 inUV0;
layout(location = 3) in vec3 inNormal;

layout(set = 0, binding = 0) uniform RawLighting
{
    vec3 sunDir;
    vec3 sunCol;
    vec3 camPos;
} world;

layout(set = 1, binding = 0) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;

const float ambientLight = 0.1;
const vec3 lightColor = vec3(1.0, 1.0, 1.0);

void main()
{
    vec2 realUV = inUV0;

    vec3 norm = normalize(inNormal);
    vec3 lighDir = normalize(world.sunDir);
    float diff = max(dot(norm, lighDir), 0.0);
    vec3 diffuse = diff * world.sunCol;
    

    outColor = texture(tex, realUV) * vec4(ambientLight + diffuse, 1.0);
}