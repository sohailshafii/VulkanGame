
#include "CommonMath.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 CommonMath::ConstructProjectionMatrix(uint32_t width, uint32_t height) {
	glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f),
		width / (float)height, nearPlaneDistance,
		farPlaneDistance);
	projectionMat[1][1] *= -1; // flip Y -- opposite of opengl
	return projectionMat;
}

glm::vec3 CommonMath::GetCartesianFromSphericalCoords(float azimRadians,
	float polarRadians, float radius) {
	float rTimesSinPolar = radius * sin(polarRadians);
	return glm::vec3(cos(azimRadians) * rTimesSinPolar, radius * cos(polarRadians),
		sin(azimRadians) * rTimesSinPolar);
}

CommonMath::Quaternion CommonMath::GetQuaternionForSphericalCoords(glm::mat4 const & matrixTrans) {
	CommonMath::Quaternion result;
	result.r = 0.5f * sqrt(1.0f + matrixTrans[0][0] + matrixTrans[1][1]
		+ matrixTrans[2][2]);
	float denom = 4.0f * result.r;
	result.i = (matrixTrans[2][1] - matrixTrans[1][2]) / denom;
	result.j = (matrixTrans[0][2] - matrixTrans[2][0]) / denom;
	result.k = (matrixTrans[1][0] - matrixTrans[0][1]) / denom;
	return result;
}

void CommonMath::CreateCoordinateSystemForLookDir(glm::vec3 const& lookAtDir,
	glm::vec3& up, glm::vec3& right) {
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	if (glm::dot(up, lookAtDir) > 0.99f) {
		up = glm::vec3(-0.2f, 0.8f, 0.0f);
		up = glm::normalize(up);
	}
	right = glm::cross(up, lookAtDir);
	// make sure up is perpendicular to both right and up
	up = glm::cross(lookAtDir, right);
}
