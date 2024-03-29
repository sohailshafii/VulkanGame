#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 worldNormal;
layout(location = 3) in vec3 worldPos;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(0.76f, 0.70f, 0.50f, 1.0f);//vec4(
		//fragColor*texture(texSampler, fragTexCoord).rgb, 1.0);
}
