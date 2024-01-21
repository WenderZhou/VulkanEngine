#version 450

layout (location = 0) in vec2 fragOffset;

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

layout(push_constant) uniform Push {
  vec4 position;
  vec4 color;
  float radius;
} push;

const float M_PI = 3.1415926535f;

void main()
{
	float dist2 = dot(fragOffset, fragOffset);
	if(dist2 > 1.0f)
	{
		discard;
	}
	outColor = vec4(push.color.xyz, 0.5f * cos(dist2 * M_PI) + 0.5f);
}