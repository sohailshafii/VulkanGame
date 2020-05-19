#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	float time;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 worldNormal;
layout(location = 3) out vec3 worldPos;

// https://catlikecoding.com/unity/tutorials/flow/waves/
vec3 GerstnerWave (vec4 wave, vec3 p, inout vec3 tangent, inout vec3 binormal)
{
	float steepness = wave.z;
	float wavelength = wave.w;
	float k = 2.0 * 3.14159 / wavelength;
	float c = sqrt(9.8/k);
	vec2 d = normalize(wave.xy);
	float f = k*(dot(d, p.xz) - c * ubo.time);

	float a = steepness / k;

	tangent += vec3(
		-d.x * d.x * (steepness * sin(f)),
		d.x * (steepness * cos(f)),
		-d.x * d.y * (steepness * sin(f)));
	binormal += vec3(
		-d.x * d.y * (steepness * sin(f)),
		d.y * (steepness * cos(f)),
		-d.y * d.y * (steepness * sin(f))
		);
	return vec3(
		d.x * (a * cos(f)),
		a * sin(f),
		d.y * (a * cos(f))
	);
}

void main() {
	gl_Position = ubo.proj * ubo.view *
		ubo.model * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;

	worldPos = vec3(ubo.model * vec4(inPosition, 1.0));
	worldNormal = mat3(ubo.model) * inNormal;
}

