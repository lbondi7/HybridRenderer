#pragma once

#include "Constants.h"
#include "Buffer.h"
#include "Device.h"
#include "SwapChain.h"
#include "Window.h"
#include "VulkanCore.h"
#include "Resources.h"
#include "Camera.h"
#include "Scene.h"
#include "AccelerationStructure.h"
#include "RayTracingPipeline.h"

struct RayTracingScratchBuffer
{
	uint64_t deviceAddress = 0;
	VkBuffer handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

// Ray tracing acceleration structure
struct AccelerationStructure2 {
	VkAccelerationStructureKHR handle;
	uint64_t deviceAddress = 0;
	VkDeviceMemory memory;
	VkBuffer buffer;
};


class RayTracingRenderer
{
public:
	RayTracingRenderer() = default;
	RayTracingRenderer(VulkanCore* core, Window* _window);
	~RayTracingRenderer();

	void Initialise(DeviceContext* _deviceContext, Window* _window, SwapChain* swapChain, Resources* _resources);

	void Deinitialise();

	void GetCommandBuffers(uint32_t imageIndex, std::vector<VkCommandBuffer>& submitCommandBuffers, Scene* scene);

	void createStorageImage();

	void CreateShaderBindingTable();

	void CreateDescriptorSets();

	void CreateRayTracingPipeline();

	void createUniformBuffer();

	void Reinitialise();

	void buildCommandBuffers(Scene* scene);

	void updateUniformBuffers(Camera* camera);

	PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
	PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;

	Buffer raygenShaderBindingTable;
	Buffer missShaderBindingTable;
	Buffer hitShaderBindingTable;

	Texture storageImage;

	struct UniformData {
		glm::mat4 viewInverse;
		glm::mat4 projInverse;
	} uniformData;

	Buffer ubo;

	RayTracingPipeline rtPipeline;
	Descriptor rtDescriptor;

	DeviceContext* deviceContext;
	SwapChain* swapChain;
	Window* window;
	Resources* resources;
	std::vector<VkCommandBuffer> drawCmdBuffers;

	bool commandBuffersReady = false;

};

