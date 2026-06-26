#version 450

layout(location = 1) in vec2 inUV0;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inFragPos;

layout(push_constant) uniform PushConstants {
    layout(offset = 64) vec3 sunDir;
    vec3 camPos;
} world;

layout(binding = 0) uniform sampler2D tex;

// r = norm.x | g = norm.y | b = norm.z | a = roughness
layout(binding = 1) uniform sampler2D pbr;

layout(location = 0) out vec4 outColor;

const float ambientLight = 0.1;
const float specularStrength = 0.5;
const vec3 lightColor = vec3(1.0, 1.0, 1.0);

vec3 CalcDiffuse(vec3 normal, vec3 lightDir)
{
    float diff = max(dot(normal, lightDir), 0.0);
    return diff * lightColor;
}

vec3 CalcSpecular(vec3 normal, vec3 lightDir, float rough)
{
    vec3 viewDir = normalize(world.camPos - inFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    return specularStrength * spec * lightColor * (1 - rough);
}

vec3 CalcWorldNormal(vec3 N, vec3 tangentNormal)
{
    vec3 pos_dx = dFdx(inFragPos);
    vec3 pos_dy = dFdy(inFragPos);
    vec2 uv_dx  = dFdx(inUV0);
    vec2 uv_dy  = dFdy(inUV0);

    vec3 T = normalize(pos_dx * uv_dy.t - pos_dy * uv_dx.t);
    T = normalize(T - N * dot(N, T));
    vec3 B = cross(N, T);

    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

void main()
{
    vec4 pbrSample = texture(pbr, inUV0);
    vec3 tangentNormal = normalize(pbrSample.xyz * 2.0 - 1.0);
    float rough = pbrSample.w;

    vec3 N = normalize(inNormal);
    vec3 normal = CalcWorldNormal(N, tangentNormal);
    
    vec3 lightDir = normalize(world.sunDir);

    vec3 diffuse = CalcDiffuse(normal, lightDir);
    vec3 specular = CalcSpecular(normal, lightDir, rough);

    vec3 pbrResult = ambientLight + diffuse + specular;
    outColor = texture(tex, inUV0) * vec4(pbrResult, 1.0);
    //outColor = vec4(normal, 1.0);
}