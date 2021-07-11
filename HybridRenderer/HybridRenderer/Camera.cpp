#include "Camera.h"
#include "Initilizers.h"

Camera::~Camera()
{

}

void Camera::init(DeviceContext* deviceContext, const VkExtent2D& _extent)
{
	viewport = Initialisers::viewport(x, y, static_cast<float>(_extent.width),  static_cast<float>(_extent.height));

	scissor = Initialisers::scissor(_extent);


	auto imageCount = deviceContext->imageCount;


	buffers.resize(imageCount);
	for (size_t i = 0; i < imageCount; i++) {
		VkDeviceSize bufferSize = sizeof(CameraUBO);
		buffers[i].Create2(deviceContext, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	using BindingType = std::pair<uint32_t, VkDescriptorType>;
	DescriptorSetRequest request;
	request.ids.emplace_back(BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));

	request.data.reserve(imageCount);
	for (size_t i = 0; i < imageCount; i++) {

		request.data.push_back(&buffers[i].descriptorInfo);
	}

	deviceContext->getDescriptors(descriptor, request);
	//descriptor.initialise(deviceContext, request);
}

void Camera::update(float windowWidth, float windowHeight)
{
	if (!valuesUpdated(windowWidth, windowHeight))
	{
		transform.getMatrix(model);

		view = glm::lookAt(transform.position, lookAtPlace ? lookAt : transform.position + transform.forward, worldUp);

		projection = glm::perspective(glm::radians(FOV), windowWidth / windowHeight, nearPlane, farPlane);
		projection[1][1] *= -1;
		updateValues(windowWidth, windowHeight);
	}
}

void Camera::update(const VkExtent2D& _extent)
{
	if (!valuesUpdated(_extent))
	{
		extent = _extent;
		transform.getMatrix(model);

		view = glm::lookAt(transform.position, lookAtPlace ? lookAt : transform.position + transform.forward, worldUp);

		projection = glm::perspective(glm::radians(FOV), static_cast<float>(extent.width) /static_cast<float>(extent.height), nearPlane, farPlane);
		projection[1][1] *= -1;

		viewport.x = x;
		viewport.y = y;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);

		scissor.extent = extent;

		updateValues(extent);
	}
}

bool Camera::valuesUpdated(const VkExtent2D& _extent) {
	return prevTransform == transform &&
		prevLookAt == lookAt && prevUp == worldUp && lookAtPlace == prevLookAtPlace &&
		prevFOV == FOV && prevNearPlane == nearPlane && prevFarPlane == farPlane &&
		extent.width == _extent.width && extent.height == _extent.height;
}

void Camera::setViewport(VkCommandBuffer cmdBuffer)
{
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
					
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
}

bool Camera::valuesUpdated(float windowWidth, float windowHeight) {
	return prevTransform == transform &&
		prevLookAt == lookAt && prevUp == worldUp && lookAtPlace == prevLookAtPlace &&
		prevFOV == FOV && prevNearPlane == nearPlane && prevFarPlane == farPlane &&
		prevWindowWidth == windowWidth && prevWindowHeight == windowHeight;
}

void Camera::updateValues(const VkExtent2D& _extent) {

	prevTransform = transform;
	prevLookAt == lookAt;
	prevUp = worldUp;
	prevFOV = FOV;
	prevNearPlane = nearPlane;
	prevFarPlane = farPlane;
	extent = _extent;
	prevLookAtPlace = lookAtPlace;
}

void Camera::updateValues(float windowWidth, float windowHeight) {

	prevTransform = transform;
	prevLookAt == lookAt;
	prevUp = worldUp;
	prevFOV = FOV;
	prevNearPlane = nearPlane;
	prevFarPlane = farPlane;
	prevWindowWidth = windowWidth;
	prevWindowHeight = windowHeight;
	prevLookAtPlace = lookAtPlace;
}
