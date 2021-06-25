#pragma once

#include "Constants.h"
#include "Transform.h"

class Camera
{
public:
	Camera() = default;
	~Camera();

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 model;


	Transform transform;
	glm::vec3 lookAt = glm::vec3(0, 0, 0);
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec3 forward = glm::vec3(0);
	glm::vec3 right = glm::vec3(0);

	float FOV = 45.0f;
	float nearPlane = 1.0f;
	float farPlane = 1000.0f;

	bool lookAtPlace = false;

	void update(float windowWidth, float windowHeight);

private:
	bool valuesUpdated(float windowWidth, float windowHeight);
	void updateValues(float windowWidth, float windowHeight);


	Transform prevTransform;
	glm::vec3 prevLookAt = glm::vec3(0, 0, 0);
	glm::vec3 prevUp = glm::vec3(0, 1, 0);

	float prevFOV;
	float prevNearPlane;
	float prevFarPlane;

	float prevWindowWidth;
	float prevWindowHeight;

	bool prevLookAtPlace;
};