
#pragma once

#include <cmath>
#include <glm/glm.hpp>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

class CommonMath {
public:
	static constexpr float nearPlaneDistance = 0.1f;
	static constexpr float farPlaneDistance = 1000.0f;

	class Quaternion {
		public:
			float i, j, k, r;
	};

	static glm::mat4 ConstructProjectionMatrix(uint32_t width, uint32_t height);
	static glm::vec3 GetCartesianFromSphericalCoords(float azimRadians, float polarRadians, float radius);
	static Quaternion GetQuaternionForMatrix(glm::mat4 const& matrixTrans);
	static Quaternion GetNormalizedQuaternion(Quaternion const& quat);
	static glm::mat4 GetMatrixForQuaternion(Quaternion const& quat);
	static void CreateCoordinateSystemForLookDir(glm::vec3 const& lookAtDir,
		glm::vec3& up, glm::vec3& right);

	static Quaternion Slerp(Quaternion const& q1, Quaternion const& q2, float t);
};
