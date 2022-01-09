#include "Camera.h"
#include "Initilizers.h"

#include "ImGUI_.h"

#include <algorithm>

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
		VkDeviceSize bufferSize = sizeof(CameraGPU);
		buffers[i].Allocate(deviceContext, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	DescriptorSetRequest request({ { "scene", 0 } });
	request.AddDescriptorBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	request.AddDescriptorBufferData(0, buffers.data());
	deviceContext->GetDescriptors(descriptor, &request);
	gpuData.rayCullDistance = 0.0f;
	adaptiveDistance = true;
	transform.position.y = 4.0f;
	widget.enabled = true;
	//lookAtPlace = true;
	lookAt = glm::vec3(0.0f, 1.0f, 0.0f);
	transform.position = glm::vec3(0.0f, 5.0f, 10.0f);
	update(_extent.width, _extent.height);
	//distances.reserve(1000000);

	float length = 60.0f;

	positions = {glm::vec3(50, 25, 80), glm::vec3(0, 1, 45), glm::vec3(-30, 1, 0), glm::vec3(-45, 15, 20)};
	auto d1 = glm::distance(positions[0], positions[1]);
	auto d2 = glm::distance(positions[1], positions[2]);
	auto d3 = glm::distance(positions[2], positions[3]);
	auto total = d1 + d2 + d3;
	timings.emplace_back(length * (d1 / total));
	timings.emplace_back(length * (d2 / total));
	timings.emplace_back(length * (d3 / total));

}

void Camera::update(float windowWidth, float windowHeight)
{
	if (!valuesUpdated(windowWidth, windowHeight))
	{
		transform.getMatrix(model);

		auto view = glm::lookAt(transform.position, lookAtPlace ? lookAt : transform.position + transform.forward, worldUp);

		auto projection = glm::perspective(glm::radians(FOV), windowWidth / windowHeight, nearPlane, farPlane);
		//projection[1][1] *= -1;
		//updateValues(windowWidth, windowHeight);

		//frustum.update(projection * view);

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

		auto view = glm::lookAt(transform.position, lookAtPlace ? lookAt : transform.position + transform.forward, worldUp);

		auto projection = glm::perspective(glm::radians(FOV), vkViewport.width / vkViewport.height, nearPlane, farPlane);
		projection[1][1] *= -1;
		gpuData.viewProjection = projection * view;
		updateValues(extent);
	}
}

void Camera::update(float dt)
{
	if (pan) {

		time += dt;

		transform.position = glm::lerp(positions[currentIndex], positions[currentIndex + 1], time/timings[currentIndex]);

		if (time / timings[currentIndex] >= 1.0) {
			if (currentIndex == 2) {
				currentIndex = 0;
				pan = false;
				time = 0.0f;
			}
			else {
				currentIndex++;
				time = 0.0f;
			}
		}

	}

	if (!valuesUpdated(extent))
	{
		transform.getMatrix(model);

		auto view = glm::lookAt(transform.position, lookAtPlace ? lookAt : transform.position + transform.forward, worldUp);
		auto projection = glm::perspective(glm::radians(FOV), vkViewport.width / vkViewport.height, nearPlane, farPlane);
		//auto aspectRatio = vkViewport.width / vkViewport.height;
		//auto projection = glm::ortho(-aspectRatio * zoom, aspectRatio * zoom, 
		//	-1.0f * zoom, 1.0f * zoom, nearPlane, farPlane);
		projection[1][1] *= -1;
		gpuData.viewProjection = projection * view;
		gpuData.position = transform.position;

		updateValues(extent);
	}

	if (ImGUI::enabled && widget.enabled) {
		if (widget.NewWindow("Camera"))
		{

			widget.Vec3("Position", transform.position);

			widget.Slider("FOV", &FOV, 1.0f, 179.0f);

			widget.Slider("Ray Query Cull Distance", &gpuData.rayCullDistance, 0.0f, 100.0f);

			widget.Slider("Zoom", &zoom, 0.0f, 100.0f);

			widget.CheckBox("Adaptive Distance", &adaptiveDistance);

			widget.CheckBox("Look At", &lookAtPlace);

			//widget.Slider4("Viewport", viewport, 0.0f, 1.0f);

			//widget.Slider4("Scissor Rect", scissor, 0.0f, 1.0f);

		}
		widget.EndWindow();
	}
	//distances.emplace_back(gpuData.rayCullDistance);
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

void Camera::SetCullDistance(float cullDistance)
{
	gpuData.rayCullDistance = std::clamp(cullDistance, 5.0f, 100.0f);
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

void Camera::ResetPan() {

	currentIndex = 0;
	pan = true;
	time = 0.0f;
}