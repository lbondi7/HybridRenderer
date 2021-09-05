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

	void Initialise(DeviceContext* _deviceContext, Window* _window, SwapChain* swapChain, Resources* _resources, Scene* scene);

	void cleanup();

	void GetCommandBuffers(uint32_t imageIndex, std::vector<VkCommandBuffer>& submitCommandBuffers);

	void Render(Camera* camera);

	void createStorageImage();

	void CreateShaderBindingTable();

	void CreateDescriptorSets();

	void CreateRayTracingPipeline();

	void createUniformBuffer();

	void Reinitialise();

	void buildCommandBuffers();

	void updateUniformBuffers(Camera* camera);

	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
	PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
	PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
	PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
	PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
	PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;

	std::vector<AccelerationStructure> blas;
	AccelerationStructure tlas;

	uint32_t indexCount;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
	Buffer raygenShaderBindingTable;
	Buffer missShaderBindingTable;
	Buffer hitShaderBindingTable;

	Texture storageImage;

	struct UniformData {
		glm::mat4 viewInverse;
		glm::mat4 projInverse;
	} uniformData;

	Buffer ubo;

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;

	DeviceContext* deviceContext;
	SwapChain* swapChain;
	Window* window;
	Resources* resources;
	std::vector<VkCommandBuffer> drawCmdBuffers;

	//Camera camera;

	bool resized = false;

	//size_t currentFrame = 0;
	//uint32_t imageIndex;

	//std::vector<VkSemaphore> nextImageSemaphores;
	//std::vector<VkSemaphore> presentSemphores;
	//std::vector<VkFence> inFlightFences;
	//std::vector<VkFence> imagesInFlight;
};

