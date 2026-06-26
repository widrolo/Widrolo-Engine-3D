#version 450

layout(location = 4) in vec3 inFragPos;

layout(location = 0) out vec4 outColor;

const float radius = 400.0;
const vec3 skyColor = vec3(0.53, 0.81, 0.92);
const vec3 voidColor = vec3(0.90, 0.90, 0.90);

void main()
{
	float yPos = inFragPos.y;
	float bias = -yPos / radius + 0.5;
	float biasVoid = yPos / radius + 0.25;

	vec3 topColor = skyColor * bias;
	topColor.x = max(min(topColor.x, skyColor.x), 0.0);
	topColor.y = max(min(topColor.y, skyColor.y), 0.0);
	topColor.z = max(min(topColor.z, skyColor.z), 0.0);

	vec3 bottomColor = voidColor * biasVoid;
	bottomColor.x = max(min(bottomColor.x, voidColor.x), 0.0);
	bottomColor.y = max(min(bottomColor.y, voidColor.y), 0.0);
	bottomColor.z = max(min(bottomColor.z, voidColor.z), 0.0);


	outColor = vec4(bottomColor + topColor, 1.0);
}