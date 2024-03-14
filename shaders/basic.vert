#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texcoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec3 fragWorldNormal;
layout(location = 3) out vec2 fragTexCoord;

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

layout(push_constant) uniform Push
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main()
{
	vec4 wPos = push.modelMatrix * vec4(position, 1.0);

	gl_Position = ubo.project * ubo.view * wPos;

	fragColor = color;
	fragWorldPos = wPos.xyz;
	fragWorldNormal = normalize(mat3(push.normalMatrix) * normal);
	fragTexCoord = texcoord;
}