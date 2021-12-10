
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

// https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
CommonMath::Quaternion CommonMath::GetQuaternionForMatrix(glm::mat4 const & matrixTrans) {
	CommonMath::Quaternion result;
	result.r = 0.5f * sqrt(1.0f + matrixTrans[0][0] + matrixTrans[1][1]
		+ matrixTrans[2][2]);
	float denom = 4.0f * result.r;
	result.i = (matrixTrans[2][1] - matrixTrans[1][2]) / denom;
	result.j = (matrixTrans[0][2] - matrixTrans[2][0]) / denom;
	result.k = (matrixTrans[1][0] - matrixTrans[0][1]) / denom;
	return result;
}

CommonMath::Quaternion CommonMath::GetNormalizedQuaternion(CommonMath::Quaternion const& quat) {
	Quaternion unitQuat = quat;
	float magnitude = sqrt(quat.i * quat.i + quat.j * quat.j + quat.k * quat.k + quat.r * quat.r);
	unitQuat.i /= magnitude;
	unitQuat.j /= magnitude;
	unitQuat.k /= magnitude;
	unitQuat.r /= magnitude;

	return unitQuat;
}

// https://songho.ca/opengl/gl_quaternion.html
glm::mat4 CommonMath::GetMatrixForQuaternion(CommonMath::Quaternion const& quat) {
	glm::mat4 result = glm::mat4(1.0f);auto normQuat = GetNormalizedQuaternion(quat);

	result[0][0] = 1.0f - 2.0f * normQuat.j * normQuat.j - 2.0f * normQuat.k * normQuat.k;
	result[0][1] = 2.0f * normQuat.i * normQuat.j - 2.0f * normQuat.r * normQuat.k;
	result[0][2] = 2.0f * normQuat.i * normQuat.k + 2.0f * normQuat.r * normQuat.j;

	result[1][0] = 2.0f * normQuat.i * normQuat.j + 2.0f * normQuat.r * normQuat.k;
	result[1][1] = 1.0f - 2.0f * normQuat.i * normQuat.i - 2.0f * normQuat.k * normQuat.k;
	result[1][2] = 2.0f * normQuat.j * normQuat.k - 2.0f * normQuat.r * normQuat.i;

	result[2][0] = 2.0f * normQuat.i * normQuat.k - 2.0f * normQuat.r * normQuat.j;
	result[2][1] = 2.0f * normQuat.j * normQuat.k + 2.0f * normQuat.r * normQuat.i;
	result[2][2] = 1.0f - 2.0f * normQuat.i * normQuat.i - 2.0f * normQuat.j * normQuat.j;

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

// http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm
CommonMath::Quaternion CommonMath::Slerp(CommonMath::Quaternion const& q1,
	CommonMath::Quaternion const& q2, float t) {
	CommonMath::Quaternion result;

	float cosHalfTheta = q1.i * q2.i + q1.j * q2.j + q1.k * q2.k + q1.r * q2.r;
	// if q1=q2 or q1=-q2 then theta = 0 and we can return q1
	if (fabs(cosHalfTheta) >= 1.0f) {
		result = q1;
		return result;
	}
	float halfTheta = acos(cosHalfTheta);
	float sinHalfTheta = sqrt(1.0f - cosHalfTheta * cosHalfTheta);
	// theta = 180 => not fully defined
	// we could rotation around any axis normal to qa or qb
	if (fabs(sinHalfTheta) < 0.001) { // fabs is floating point absolute
		result.i = (q1.i * 0.5f + q2.i * 0.5f);
		result.j = (q1.j * 0.5f + q2.j * 0.5f);
		result.k = (q1.k * 0.5f + q2.k * 0.5f);
		result.r = (q1.r * 0.5f + q2.r * 0.5f);
		return result;
	}

	float ratioA = sin((1.0f - t) * halfTheta) / sinHalfTheta;
	float ratioB = sin(t * halfTheta) / sinHalfTheta;
	//calculate Quaternion.
	result.i = (q1.i * ratioA + q2.i * ratioB);
	result.j = (q1.j * ratioA + q2.j * ratioB);
	result.k = (q1.k * ratioA + q2.k * ratioB);
	result.r = (q1.r * ratioA + q2.r * ratioB);
	return result;
}
