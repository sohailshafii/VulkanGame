#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

// TODO: put other variables here for lambertian
layout(binding = 0) uniform UniformBufferObject {
	vec3 lightPosition;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 worldNormal;
layout(location = 3) in vec3 worldPos;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(1.0, 0.0, 0.0, 1.0);
	//vec4(
	//	fragColor*texture(texSampler, fragTexCoord).rgb, 1.0);
}
