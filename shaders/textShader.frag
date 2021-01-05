#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform UniformBufferObject {
	vec4 colorTint;
} ubo;

layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
	float textSample = texture(texSampler, texCoords).r;
	outColor = fragColor*vec4(textSample.r, textSample.r, textSample.r,
		textSample.r);
}
