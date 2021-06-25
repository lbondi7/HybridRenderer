#include "Camera.h"


Camera::~Camera()
{
}

void Camera::update(float windowWidth, float windowHeight)
{
	if (!valuesUpdated(windowWidth, windowHeight))
	{
		transform.getMatrix(model);

		view = glm::lookAt(transform.position, lookAtPlace ? lookAt : transform.position + transform.forward, up);

		projection = glm::perspective(glm::radians(FOV), windowWidth / windowHeight, nearPlane, farPlane);
		projection[1][1] *= -1;
		updateValues(windowWidth, windowHeight);
	}
}

bool Camera::valuesUpdated(float windowWidth, float windowHeight) {
	return prevTransform == transform &&
		prevLookAt == lookAt && prevUp == up && lookAtPlace == prevLookAtPlace &&
		prevFOV == FOV && prevNearPlane == nearPlane && prevFarPlane == farPlane &&
		prevWindowWidth == windowWidth && prevWindowHeight == windowHeight;
}

void Camera::updateValues(float windowWidth, float windowHeight) {

	prevTransform = transform;
	prevLookAt == lookAt;
	prevUp = up;
	prevFOV = FOV;
	prevNearPlane = nearPlane;
	prevFarPlane = farPlane;
	prevWindowWidth = windowWidth;
	prevWindowHeight = windowHeight;
	prevLookAtPlace = lookAtPlace;
}
