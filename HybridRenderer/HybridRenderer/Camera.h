#pragma once

#include "Constants.h"
#include "Transform.h"
#include "Buffer.h"

#include "Descriptor.h"

#include "Frustum.hpp"

#include "ImGUIWidgets.h"


struct CameraGPU {
	alignas(16) glm::mat4 viewProjection;
	alignas(16) glm::vec3 position;
	float rayCullDistance;
};

class Camera
{
public:
	Camera() = default;
	~Camera();

	//glm::mat4 view;
	//glm::mat4 projection;
	glm::mat4 model;


	Transform transform;
	glm::vec3 lookAt = glm::vec3(0, 0, 0);
	glm::vec3 worldUp = glm::vec3(0, 1, 0);
	glm::vec3 worldForward = glm::vec3(0);
	glm::vec3 worldRight = glm::vec3(0);

	VkViewport vkViewport;
	VkRect2D vkScissor;
	glm::vec4 scissor = glm::vec4(0, 0, 1, 1);
	glm::vec4 viewport = glm::vec4(0, 0, 1, 1);

	VkExtent2D extent;

	float FOV = 45.0f;
	float nearPlane = 1.0f;
	float farPlane = 1000.0f;

	bool lookAtPlace = false;

	void init(DeviceContext* deviceContext, const VkExtent2D& _extent);

	void update(float windowWidth, float windowHeight);

	void update(const VkExtent2D& _extent);

	void update(float dt);

	bool valuesUpdated(const VkExtent2D& _extent);

	void vkSetViewport(VkCommandBuffer cmdBuffer);

	void setViewport(glm::vec2 size, glm::vec2 offset);

	void setScissor(glm::vec2 size, glm::vec2 offset);

	void SetCullDistance(float cullDistance);


	std::vector<Buffer> buffers;

	Descriptor descriptor;

	void updateWindow(float windowWidth, float windowHeight);

	void ResetPan();

	Frustum frustum;

	ImGUIWidget widget;

	CameraGPU gpuData;
	bool adaptiveDistance = true;
	float multiplier = 5.0f;
	std::vector<float> distances;

	bool pan = false;
	std::vector<glm::vec3> positions;
	std::vector<float> timings;
	float time = 0.0;
	size_t currentIndex = 0;
	float zoom = 1.0f;

private:
	bool valuesUpdated(float windowWidth, float windowHeight);
	void updateValues(const VkExtent2D& _extent);


	float windowWidth = 0;
	float windowHeight = 0;

	Transform prevTransform;
	glm::vec3 prevLookAt = glm::vec3(0, 0, 0);
	glm::vec3 prevUp = glm::vec3(0, 1, 0);

	glm::vec4 prevScissor = glm::vec4(0, 0, 1, 1);
	glm::vec4 prevViewport = glm::vec4(0, 0, 1, 1);

	float prevFOV;
	float prevNearPlane;
	float prevFarPlane;

	float prevWindowWidth;
	float prevWindowHeight;

	bool prevLookAtPlace;
	float distance = 10.0f;
	float angle = 0.0f;
	float speed = 0.2f;

};