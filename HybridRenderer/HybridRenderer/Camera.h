#pragma once

#include "Constants.h"
#include "Transform.h"
#include "Buffer.h"

#include "Descriptor.h"


struct CameraUBO {
	alignas(16) glm::mat4 projection;
	alignas(16) glm::mat4 view;
	alignas(16) glm::vec3 camPos;
};

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
	glm::vec3 worldUp = glm::vec3(0, 1, 0);
	glm::vec3 worldForward = glm::vec3(0);
	glm::vec3 worldRight = glm::vec3(0);

	VkViewport viewport;
	VkRect2D scissor;

	VkExtent2D extent;

	float FOV = 45.0f;
	float nearPlane = 1.0f;
	float farPlane = 1000.0f;

	float x = 0.0f;
	float y = 0.0f;

	bool lookAtPlace = false;

	void init(DeviceContext* deviceContext, const VkExtent2D& _extent);

	void update(float windowWidth, float windowHeight);

	void update(const VkExtent2D& _extent);

	bool valuesUpdated(const VkExtent2D& _extent);

	void setViewport(VkCommandBuffer cmdBuffer);

	std::vector<Buffer> buffers;

	Descriptor descriptor;

	std::vector<VkDescriptorSet> cameraDescSets;


private:
	bool valuesUpdated(float windowWidth, float windowHeight);
	void updateValues(const VkExtent2D& _extent);
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