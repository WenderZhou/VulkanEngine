#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragWorldPos;
layout (location = 2) in vec3 fragWorldNormal;

layout (location = 0) out vec4 outColor;

struct PointLight
{
	vec4 position;
	vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 project;
	mat4 view;
	vec4 ambientLightColor;
	PointLight pointLights[10];
	int numLights;
} ubo;

layout(push_constant) uniform Push
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main()
{
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 surfaceNormal = normalize(fragWorldNormal);

	for(int i = 0; i < ubo.numLights; i++)
	{
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xyz - fragWorldPos;
		float attenuation = 1.0f / dot(directionToLight, directionToLight);
		directionToLight = normalize(directionToLight);

		vec3 lightColor = light.color.xyz * light.color.w * attenuation;
		diffuseLight += lightColor * max(dot(surfaceNormal, directionToLight), 0);
	}

	outColor = vec4(fragColor * diffuseLight, 1.0);
}