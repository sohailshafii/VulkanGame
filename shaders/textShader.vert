#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inPosition;

layout(location = 0) out vec2 TexCoords;

layout(binding = 0) uniform UniformBufferObject {
	mat4 projection;
} ubo;

void main() {
	gl_Position = ubo.projection * vec4(inPosition.xy, 0.0, 1.0);
	TexCoords = inPosition.zw;
}
