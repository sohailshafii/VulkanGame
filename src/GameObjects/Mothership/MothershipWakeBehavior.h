#pragma once

#include "ShipStateBehavior.h"
#include <glm/glm.hpp>

class MothershipWakeBehavior : public ShipStateBehavior {
public:
	MothershipWakeBehavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior & motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "MothershipWakeBehavior";
	}

private:
	void InitializeIfRequired(MothershipBehavior const& motherShip,
		float time);

	static inline const glm::vec3 startOffset = glm::vec3(0.0f, 50.0f, -50.0f);
	static constexpr float spawnDuration = 4.0f;

	float wakeTime;
	glm::mat4 modelMatrix;
	glm::vec3 originalMothershipPosition;
};
