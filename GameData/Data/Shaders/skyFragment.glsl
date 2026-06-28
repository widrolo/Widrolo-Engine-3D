#version 450

layout(location = 4) in vec3 inFragPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform RawLighting
{
    vec3 sunDir;
    vec3 sunCol;
	float ambIntensity;
	vec3 ambCol;
    vec3 camPos;
	float time;
} world;

const float radius = 400.0;
const vec3 skyColor = vec3(0.53, 0.81, 0.92);
const vec3 voidColorDay = vec3(0.90, 0.90, 0.90);
const vec3 voidColorNight = vec3(0.008, 0.051, 0.188);

#define PI 3.1415926538

vec3 CalcTimedSkyColor(float fac)
{
	return skyColor * fac;
}

vec3 CalcTimedVoidColor(float fac)
{
	return mix(voidColorNight, voidColorDay, fac);
}

float CalcVoidBias(float fac)
{
	float timeFac = (-fac / 4) + 0.55;

	return inFragPos.y / radius + timeFac;
}

void main()
{
	float yPos = inFragPos.y;
	

	float timeFac = tanh(2*sin((world.time - 2*PI) * 2*PI)) / 2.1 + (0.524);

	vec3 realSky = CalcTimedSkyColor(timeFac);
	vec3 realVoid = CalcTimedVoidColor(timeFac);

	float bias = -yPos / radius + 0.5;
	float biasVoid = CalcVoidBias(timeFac);

	vec3 topColor = realSky * world.sunCol * bias;
	topColor.x = max(min(topColor.x, realSky.x), 0.0);
	topColor.y = max(min(topColor.y, realSky.y), 0.0);
	topColor.z = max(min(topColor.z, realSky.z), 0.0);

	vec3 bottomColor = realVoid * biasVoid;
	bottomColor.x = max(min(bottomColor.x, realVoid.x), 0.0);
	bottomColor.y = max(min(bottomColor.y, realVoid.y), 0.0);
	bottomColor.z = max(min(bottomColor.z, realVoid.z), 0.0);

	outColor = vec4(bottomColor + topColor, 1.0);
}