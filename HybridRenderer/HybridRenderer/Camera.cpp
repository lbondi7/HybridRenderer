#include "Camera.h"
#include "Initilizers.h"

#include "ImGUI_.h"

Camera::~Camera()
{

}

void Camera::init(DeviceContext* deviceContext, const VkExtent2D& _extent)
{

	viewport = { 0, 0, 1, 1 };
	scissor = { 0, 0, 1, 1 };

	vkViewport = Initialisers::viewport(viewport.x, viewport.y, static_cast<float>(_extent.width) * viewport.z,  static_cast<float>(_extent.height) * viewport.w);


	vkScissor = Initialisers::scissor(_extent);
	vkScissor.offset.x = static_cast<int32_t>(scissor.x);
	vkScissor.offset.y = static_cast<int32_t>(scissor.y);
	vkScissor.extent.width = _extent.width * scissor.z;
	vkScissor.extent.height = _extent.height * scissor.w;

	auto imageCount = deviceContext->imageCount;


	buffers.resize(imageCount);
	for (size_t i = 0; i < imageCount; i++) {
		VkDeviceSize bufferSize = sizeof(CameraUBO);
		buffers[i].Allocate(deviceContext, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	using BindingType = std::pair<uint32_t, VkDescriptorType>;
	DescriptorSetRequest request;
	request.ids.emplace_back(BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));

	request.data.reserve(imageCount);
	for (size_t i = 0; i < imageCount; i++) {

		request.data.push_back(&buffers[i].descriptorInfo);
	}

	deviceContext->GetDescriptors(descriptor, request);
	//descriptor.initialise(deviceContext, request);

	update(_extent.width, _extent.height);
}

void Camera::update(float windowWidth, float windowHeight)
{
	if (!valuesUpdated(windowWidth, windowHeight))
	{
		transform.getMatrix(model);

		view = glm::lookAt(transform.position, lookAtPlace ? lookAt : transform.position + transform.forward, worldUp);

		projection = glm::perspective(glm::radians(FOV), windowWidth / windowHeight, nearPlane, farPlane);
		projection[1][1] *= -1;
		//updateValues(windowWidth, windowHeight);

		frustum.update(projection * view);

	}
}

void Camera::update(const VkExtent2D& _extent)
{
	if (!valuesUpdated(_extent))
	{
		extent = _extent;
		vkViewport.x = viewport.x;
		vkViewport.y = viewport.y;
		vkViewport.width = _extent.width * viewport.z;
		vkViewport.height = _extent.height * viewport.w;
		vkScissor.offset.x = static_cast<int32_t>(scissor.x);
		vkScissor.offset.y = static_cast<int32_t>(scissor.y);
		vkScissor.extent.width = _extent.width * scissor.z;
		vkScissor.extent.height = _extent.height * scissor.w;
		transform.getMatrix(model);

		view = glm::lookAt(transform.position, lookAtPlace ? lookAt : transform.position + transform.forward, worldUp);

		projection = glm::perspective(glm::radians(FOV), vkViewport.width / vkViewport.height, nearPlane, farPlane);
		projection[1][1] *= -1;

		updateValues(extent);
	}

	if (ImGUI::enabled && widget.enabled) {
		if (widget.NewWindow("Camera"))
		{

			widget.Slider("FOV", &FOV, 1.0f, 179.0f);

			widget.Slider4("Viewport", viewport, 0.0f, 1.0f);

			widget.Slider4("Scissor Rect", scissor, 0.0f, 1.0f);

		}
		widget.EndWindow();
	}
}

bool Camera::valuesUpdated(const VkExtent2D& _extent) {
	return prevTransform == transform &&
		prevLookAt == lookAt && prevUp == worldUp && lookAtPlace == prevLookAtPlace &&
		prevFOV == FOV && prevNearPlane == nearPlane && prevFarPlane == farPlane &&
		vkViewport.width == _extent.width && vkViewport.height == _extent.height;
}

void Camera::vkSetViewport(VkCommandBuffer cmdBuffer)
{
	vkCmdSetViewport(cmdBuffer, 0, 1, &vkViewport);
					
	vkCmdSetScissor(cmdBuffer, 0, 1, &vkScissor);
}

void Camera::setViewport(glm::vec2 size, glm::vec2 offset)
{
	viewport = {offset.x, offset.y, size.x, size.y};
	vkViewport.x = viewport.x;
	vkViewport.y = viewport.y;
	vkViewport.width = windowWidth * viewport.z;
	vkViewport.height = windowHeight * viewport.w;
}

void Camera::setScissor(glm::vec2 size, glm::vec2 offset)
{
	scissor = { offset.x, offset.y, size.x, size.y };

	vkScissor.offset.x = static_cast<int32_t>(scissor.x);
	vkScissor.offset.y = static_cast<int32_t>(scissor.y);
	vkScissor.extent.width = extent.width * scissor.z;
	vkScissor.extent.height = extent.height * scissor.w;
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
	prevViewport = viewport;
	prevScissor = scissor;
}

void Camera::updateWindow(float _windowWidth, float _windowHeight) {

	//windowWidth = _windowWidth;
	//windowHeight = _windowHeight;
	//vkViewport.x = viewport.x;
	//vkViewport.y = viewport.y;
	//vkViewport.width = windowWidth * viewport.z;
	//vkViewport.height = windowHeight * viewport.w;
	//vkScissor.offset = = { static_cast<float>(scissor.x), static_cast<float>(scissor.y) };
	//vkScissor.extent = { windowWidth * scissor.z, windowHeight * scissor.w };
}
