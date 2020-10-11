// Shader meant for mother ship object.

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	//vec4 ripplePointsLocal[10];
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

float rippleHeightValue(vec2 vertexPos, vec2 rippleCenter, float lerpValue) {
	vec2 diffVec = vertexPos - rippleCenter;
	float dotProd = dot(diffVec, diffVec);
	float heightTerm = mix(0.6, 0.01, lerpValue);
	float numeratorOffset = mix(3.0f, 30.0f, lerpValue);
	float denomOffset = mix(1.0f, 10.0f, lerpValue);

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

		vec3 currentRipplePoint = vec3(0.0, 5, 0.0);//ubo.ripplePointsLocal[i];
		vec3 distanceVec = vertexPosition - currentRipplePoint;
		vec3 vertexNormalized = normalize(vertexPosition);
		// if we are close to ripple point, then add its contribution to our
		// z value
		if (dot(distanceVec, distanceVec) < 4.0) {
			// calculate lerp value based on time
			// 0.0-0.5: lerp forward from 0 to 1, 0.5-1.0: lerp backward from 1 to 0
			// lerp from 0 to 1 to half-way point, then 1 to 0 from halfway to end
			float halfwayDuration = ubo.maxRippleDuration*0.5;
			float diffTime = ubo.time - ubo.rippleStartTime;
			float lerpVal = 0.0;
			if (diffTime < halfwayDuration) {
				lerpVal = diffTime/halfwayDuration;
				lerpVal = clamp(lerpVal, 0.0, 1.0);
			}
			else {
				lerpVal = (diffTime - halfwayDuration)/halfwayDuration;
				lerpVal = clamp(lerpVal, 0.0, 1.0);
				lerpVal = 1.0 - lerpVal;
			}

			//float newHeightVal = rippleHeightValue(vec2(vertexPosition.x,
			//	vertexPosition.y), vec2(currentRipplePoint.x, currentRipplePoint.y),
			//	lerpVal);
			vertexPosition += vertexNormalized*lerpVal*0.1;//newHeightVal;
		}
	}

	gl_Position = ubo.proj * ubo.view *
		ubo.model * vec4(vertexPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}

