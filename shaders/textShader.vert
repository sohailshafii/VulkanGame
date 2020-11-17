#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 TexCoords;

layout(binding = 0) uniform UniformBufferObject {
	mat4 projection;
} ubo;

void main() {
	gl_Position = ubo.projection * vec4(inPosition, 1.0);
	TexCoords = inTexCoord;
}
