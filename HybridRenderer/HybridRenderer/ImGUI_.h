#pragma once

#include "Constants.h"

#include "Buffer.h"
#include "Texture.h"
#include "Pipeline.h"
#include "FrameBuffer.h"

#include "imgui/imgui.h"


class ImGUI
{
public:

	static bool enabled;
	static bool firstFrame;

	static void Demo();

	ImGUI() = default;
	~ImGUI();

	//void create(DeviceContext* _devices, RenderPass* _renderPass, DescriptorSetManager* _descriptorSetManager, const PipelineInfo& _pipelineInfo);

//	void init();

	void reinit();

	//void updateBuffers();

	//void Draw(VkCommandBuffer commandBuffer, size_t cmdBufferIndex);

	void startFrame();

	void NewWindow(const char* name, bool* close);

	void endFrame();

	void deinit();


	void destroy(bool complete = true);

	bool updated = true;

	bool drawn = false;

	void update(VkExtent2D extent);
	void buildCommandBuffer(uint32_t i, const VkExtent2D& extent);
	std::vector<VkVertexInputBindingDescription> bindingDescriptions();
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions();

	void createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags);
	void createCommandBuffers(VkCommandBuffer* commandBuffer, uint32_t commandBufferCount, VkCommandPool& commandPool);
	void create(GLFWwindow* window, VkInstance instance, VkSurfaceKHR surface, DeviceContext* _devices, SwapChain* _swapChain);
	void Render();

	glm::vec2 mousePos;
	bool leftMouse = false;
	bool rightMouse = false;
	float mouseWheel = 0.0f;

	DeviceContext* devices;

	bool startedFrame = false;

	RenderPass renderPass;
	FrameBuffer frameBuffer;

	VkDescriptorPool descriptorPool;
	VkRenderPass vkRenderPass;
	VkCommandPool commandPool;
	std::vector<VkCommandPool> imGuiCommandPools;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkFramebuffer> frameBuffers;

	SwapChain* swapChain = nullptr;

	VkClearValue clearValues;

};