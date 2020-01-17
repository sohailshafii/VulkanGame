#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D textTexture;

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 textSample = vec4(1.0, 1.0, 1.0, 
		texture(textTexture, TexCoords).r);
	outColor = textSample; // TODO use a uniform color to modify this
}
