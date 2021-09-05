#pragma once

#include "Constants.h"
#include "Buffer.h"
#include "Device.h"
#include "SwapChain.h"
#include "Window.h"
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
	~RayTracingRenderer();

	void Initialise(DeviceContext* _deviceContext, VkSurfaceKHR surface, Window* _window, Resources* _resources, Scene* scene);

	void cleanup();

	void Render(Camera* camera);

	void createStorageImage();

	void CreateShaderBindingTable();

	void CreateDescriptorSets();

	void CreateRayTracingPipeline();

	void createUniformBuffer();

	void handleResize();

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


	AccelerationStructure2 bottomLevelAS{};
	std::vector<AccelerationStructure2> bottomLevelASs;
	std::vector<AccelerationStructure> blas;
	AccelerationStructure tlas;

	Buffer vertexBuffer;
	Buffer indexBuffer;
	uint32_t indexCount;
	Buffer transformBuffer;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
	Buffer raygenShaderBindingTable;
	Buffer missShaderBindingTable;
	Buffer hitShaderBindingTable;

	struct StorageImage {
		VkDeviceMemory memory;
		VkImage image;
		VkImageView view;
	} storageImage;

	struct UniformData {
		glm::mat4 viewInverse;
		glm::mat4 projInverse;
	} uniformData;

	Buffer ubo;

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSets;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;

	DeviceContext* deviceContext;
	SwapChain swapChain;
	Window* window;
	Resources* resources;
	std::vector<VkCommandBuffer> drawCmdBuffers;

	Camera camera;

	bool resized = false;

	size_t currentFrame = 0;
	uint32_t imageIndex;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
};

