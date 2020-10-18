// Shader meant for mother ship object.

#version 450
#extension GL_ARB_separate_shader_objects : enable

struct RipplePointLocal {
	vec4 ripplePosition;
	float rippleDuration;
	float rippleStartTime;	
};

struct StalkPointLocal {
	vec4 stalkPosition;
	float stalkSpawnTime;	
};

const float stalkDuration = 1.0f;

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	RipplePointLocal ripplePointsLocal[10];
	StalkPointLocal stalkPointsLocal[4];
	float time;

	float shudderStartTime;
	float shudderDuration;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

// found this cool equation here: https://www.youtube.com/watch?v=JrzgE7p-xnU
// It's basically a ripple effect
// z=(cos( 0.5sqrt(x^2+y^2)-6n)/(0.5(x^2+y^2)+1+2n),  n={0...10}
// z=cos( 0.5*sqrt(x^2+y^2)-6*2)/(0.5*(x^2+y^2)+1+2*2)
// 
// modified based on center:
// z=cos( 0.5*sqrt((x - x.c)^2+(y - y.c)^2)-6*2)/(0.5*((x - x.c)^2+(y - y.c)^2)+1+2*2)
// z=cos( 0.5*sqrt((x - x.c)^2+(y - y.c)^2)-6*n)/(0.5*((x - x.c)^2+(y - y.c)^2)+1+2*n)
// some tweaks
// z=cos( 0.6*sqrt((x - x.c)^2+(y - y.c)^2)-3)/(0.6*((x - x.c)^2+(y - y.c)^2)+1)
// z=cos( 0.6*sqrt((x)^2+(y)^2)-3)/(0.6*((x)^2+(y)^2)+1)
// final version: z=cos(heightTerm*sqrt((x - x.c)^2+(y - y.c)^2)-numeratorOffset)/(heightTerm*((x - x.c)^2+(y - y.c)^2)+denomOffset)
float rippleHeightValue(vec2 vertexPos, vec2 rippleCenter, float lerpValue) {
	vec2 diffVec = vertexPos - rippleCenter;
	float dotProd = dot(diffVec, diffVec);
	float heightTerm = mix(20, 0.001, lerpValue);
	float numeratorOffset = mix(3.0f, 90.0f, lerpValue);
	float denomOffset = mix(1.0f, 40.0f, lerpValue);

	float numerator = cos(heightTerm*sqrt(dotProd) - numeratorOffset);
	float denominator = heightTerm*dotProd + denomOffset;
	return numerator/denominator;
}

vec3 getNewVertexPositionRipple(vec3 vertexPosition) {
	vec3 offsetDirection = normalize(vertexPosition);

	for (int i = 0; i < 10; i++) {
		RipplePointLocal ripplePoint = ubo.ripplePointsLocal[i];
		float rippleStartTime = ripplePoint.rippleStartTime;
		float rippleDuration = ripplePoint.rippleDuration;
		// skip invalid ripples (-1 means "invalid")
		if (rippleStartTime < 0.0f) {
			continue;
		}

		vec3 currentRipplePoint = vec3(ripplePoint.ripplePosition);
		vec3 distanceVec = vertexPosition - currentRipplePoint;
		// if we are close to ripple point, then add its contribution to our
		// z value
		// calculate lerp value based on time
		float lerpVal = (ubo.time - rippleStartTime)/rippleDuration;
		lerpVal = clamp(lerpVal, 0.0, 1.0);

		float newHeightVal = rippleHeightValue(vec2(vertexPosition.x,
			vertexPosition.y), vec2(currentRipplePoint.x, currentRipplePoint.y),
			lerpVal);
		vertexPosition += offsetDirection*newHeightVal;
	}

	return vertexPosition;
}

// https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 shudderEffect(vec3 vertexPosition) {
	float lerpVal = (ubo.time - ubo.shudderStartTime)/ubo.shudderDuration;
	lerpVal = clamp(lerpVal, 0.0, 1.0);
	float intensity = mix(0.025f, 0.0f, lerpVal);
	vertexPosition.x += intensity*rand(vec2(ubo.time+vertexPosition.y,
		ubo.time+vertexPosition.z));
	vertexPosition.y += intensity*rand(vec2(ubo.time+vertexPosition.x,
		ubo.time+vertexPosition.z));
	vertexPosition.z += intensity*rand(vec2(ubo.time+vertexPosition.x,
		ubo.time+vertexPosition.y));

	return vertexPosition;
}

// if a vertex is too far away from a stalk, it should not be affected
// try something like z=cos(sqrt(x^2+y^2))/(3*(x^2+y^2) + 0.2)
vec3 getNewStalkPosition(vec3 vertexPosition) {
	vec3 offsetDirection = normalize(vertexPosition);

	for (int i = 0; i < 4; i++) {
		StalkPointLocal stalkPoint = ubo.stalkPointsLocal[i];
		float stalkSpawnTime = stalkPoint.stalkSpawnTime;
		// skip invalid ripples (-1 means "invalid")
		if (stalkSpawnTime < 0.0f) {
			continue;
		}

		vec3 stalkPosition = vec3(stalkPoint.stalkPosition);
		vec3 distanceVec = vertexPosition - stalkPosition;
		float dotProd = dot(distanceVec, distanceVec);
		float lerpVal = (ubo.time - stalkSpawnTime)/stalkDuration;
		lerpVal = clamp(lerpVal, 0.0, 1.0);
		float distance = sqrt(dotProd);
		float numerator = cos(distance);
		float denom = 3.0*(dotProd) + 0.2;
		float bumpIntensity = lerpVal*numerator/denom;
		// TODO: figure out function to use here
		vertexPosition += bumpIntensity*offsetDirection;
	}
	return vertexPosition;
}

void main() {
	vec3 vertexPosition = inPosition;

	if (ubo.time < (ubo.shudderStartTime + ubo.shudderDuration)) {
		vertexPosition = shudderEffect(vertexPosition);
	}

	vertexPosition = getNewVertexPositionRipple(vertexPosition);

	vertexPosition = getNewStalkPosition(vertexPosition);

	gl_Position = ubo.proj * ubo.view *
		ubo.model * vec4(vertexPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}

