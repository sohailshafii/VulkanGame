// Shader meant for mother ship object.

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	float time;
	float maxRippleDuration;
	vec3 ripplePoints[10];
	float rippleStartTime[10];
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

// found this cool equation
// TODO: use this for ripple
// z=(cos( 0.5sqrt(x^2+y^2)-6n)/(0.5(x^2+y^2)+1+2n),  n={0...10}
// z=cos( 0.5*sqrt(x^2+y^2)-6*2)/(0.5*(x^2+y^2)+1+2*2)
// https://www.youtube.com/watch?v=JrzgE7p-xnU
// modified based on center:
// z=cos( 0.5*sqrt((x - x.c)^2+(y - y.c)^2)-6*2)/(0.5*((x - x.c)^2+(y - y.c)^2)+1+2*2)
// lerp from 5 to 0.1 over time

void main() {
	gl_Position = ubo.proj * ubo.view *
		ubo.model * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}

