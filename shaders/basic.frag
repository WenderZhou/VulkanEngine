#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragWorldPos;
layout (location = 2) in vec3 fragWorldNormal;
layout (location = 3) in vec2 fragTexCoord;

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
	mat4 invView;
	vec4 ambientLightColor;
	PointLight pointLights[10];
	int numLights;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(push_constant) uniform Push
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main()
{
//	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
//	vec3 specularLight = vec3(0.0);
//	vec3 N = normalize(fragWorldNormal);
//
//	vec3 cameraWorldPos = ubo.invView[3].xyz;
//	vec3 V = normalize(cameraWorldPos - fragWorldPos);
//
//	for(int i = 0; i < ubo.numLights; i++)
//	{
//		PointLight light = ubo.pointLights[i];
//		vec3 L = light.position.xyz - fragWorldPos;
//		float attenuation = 1.0f / dot(L, L);
//		L = normalize(L);
//
//		vec3 lightColor = light.color.xyz * light.color.w * attenuation;
//		diffuseLight += lightColor * max(dot(N, L), 0);
//
//		vec3 H = normalize(L + V);
//		float NoH = clamp(dot(N, H), 0, 1);
//		specularLight += lightColor * pow(NoH, 32);
//	}
//
//	outColor = vec4(fragColor * (diffuseLight + specularLight), 1.0);
	outColor = texture(texSampler, fragTexCoord);;
}