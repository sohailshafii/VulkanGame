// Shader meant for mother ship object.

#version 450
#extension GL_ARB_separate_shader_objects : enable

struct RipplePoint {
	vec4 ripplePosition; 	
};

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	RipplePoint ripplePointsLocal[10];
	float rippleStartTime;
	float time;
	float maxRippleDuration;
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
// z=cos( 0.5*sqrt((x - x.c)^2+(y - y.c)^2)-6*n)/(0.5*((x - x.c)^2+(y - y.c)^2)+1+2*n)
// lerp from 5 to 0.1 over time

// z=cos( 0.6*sqrt((x - x.c)^2+(y - y.c)^2)-3)/(0.6*((x - x.c)^2+(y - y.c)^2)+1)
// z=cos( 0.6*sqrt((x)^2+(y)^2)-3)/(0.6*((x)^2+(y)^2)+1)

float rippleHeightValue(vec2 vertexPos, vec2 rippleCenter, float lerpValue) {
	vec2 diffVec = vertexPos - rippleCenter;
	float dotProd = dot(diffVec, diffVec);
	float heightTerm = mix(0.6, 0.001, lerpValue);
	float numeratorOffset = mix(3.0f, 90.0f, lerpValue);
	float denomOffset = mix(1.0f, 40.0f, lerpValue);

	float numerator = cos(heightTerm*sqrt(dotProd) - numeratorOffset);
	float denominator = heightTerm*dotProd + denomOffset;
	return numerator/denominator;
}

void main() {
	vec3 vertexPosition = inPosition;
	for (int i = 0; i < 10; i++) {
		// skip invalid ripples (-1 means "invalid")
		if (ubo.rippleStartTime < 0.0f) {
			continue;
		}

		RipplePoint ripplePoint = ubo.ripplePointsLocal[i];
		vec3 currentRipplePoint = vec3(ripplePoint.ripplePosition);
		vec3 distanceVec = vertexPosition - currentRipplePoint;
		vec3 offsetDirection = normalize(vertexPosition);
		// if we are close to ripple point, then add its contribution to our
		// z value
		if (dot(distanceVec, distanceVec) < 4.0) {
			// calculate lerp value based on time
			float lerpVal = (ubo.time - ubo.rippleStartTime)/ubo.maxRippleDuration;
			lerpVal = clamp(lerpVal, 0.0, 1.0);

			float newHeightVal = rippleHeightValue(vec2(vertexPosition.x,
				vertexPosition.y), vec2(currentRipplePoint.x, currentRipplePoint.y),
				lerpVal);
			vertexPosition += offsetDirection*newHeightVal;
		}
	}

	gl_Position = ubo.proj * ubo.view *
		ubo.model * vec4(vertexPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}

