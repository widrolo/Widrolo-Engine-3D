#version 450

layout(location = 1) in vec2 inUV0;
layout(location = 3) in vec3 inNormal;

layout(push_constant) uniform PushConstants {
    layout(offset = 64) vec3 dir;
} sun;

layout(binding = 0) uniform sampler2D tex;
layout(binding = 1) uniform sampler2D norm;

layout(location = 0) out vec4 outColor;

const float ambientLight = 0.1;
const vec3 lightColor = vec3(1.0, 1.0, 1.0);

void main()
{
    vec2 realUV = inUV0;

    vec3 normal = texture(norm, realUV).xyz;
    vec3 lighDir = normalize(sun.dir);
    float diff = max(dot(normal, lighDir), 0.0);
    vec3 diffuse = diff * lightColor;
    

    outColor = texture(tex, realUV) * vec4(ambientLight + diffuse, 1.0);
}