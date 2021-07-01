#pragma once

#include "Constants.h"

#include "Buffer.h"
#include "Texture.h"
#include "Pipeline.h"

#include "imgui/imgui.h"

#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"



class ImGUI
{
public:

	ImGUI() = default;
	~ImGUI();

	void create(DeviceContext* _devices, RenderPass* _renderPass, DescriptorSetManager* _descriptorSetManager, const PipelineInfo& _pipelineInfo);

	void init();

	void reinit();

	void updateBuffers();

	void Draw(VkCommandBuffer commandBuffer, size_t cmdBufferIndex);

	void newFrame();

	void startFrame();

	void NewWindow(const char* name, bool* close);

	void NewImage(VkSampler sampler, VkImageView view, VkImageLayout layout);

	void endFrame();

	void deinit();

	void destroy(bool complete = true);

	bool enabled = true;

	bool updated = true;

	void update(VkExtent2D extent);
	std::vector<VkVertexInputBindingDescription> bindingDescriptions();
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions();

	void createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags);
	void createCommandBuffers(VkCommandBuffer* commandBuffer, uint32_t commandBufferCount, VkCommandPool& commandPool);
	void create(GLFWwindow* window, VkInstance instance, VkSurfaceKHR surface, DeviceContext* _devices, SwapChain* swapChain, RenderPass* _renderPass);
	void Render();

	glm::vec2 mousePos;
	bool leftMouse = false;
	bool rightMouse = false;
	float mouseWheel = 0.0f;

	DescriptorSetManager* dsm;

	Buffer vertexBuffer;
	Buffer indexBuffer;
	int32_t vertexCount = 0;
	int32_t indexCount = 0;
	Pipeline dpipeline;
	Texture fontImage;
	//VkPipelineCache pipelineCache;
	//VkPipelineLayout pipelineLayout;
	//VkPipeline pipeline;
	//VkDescriptorPool descriptorPool;
	//VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	DeviceContext* devices;

	PipelineInfo pipelineInfo;

	PushConstBlock pushConstBlock;

	bool renderStuff = false;

	VkDescriptorPool descriptorPool;
	VkRenderPass renderPass;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer>commandBuffers;
	std::vector<VkFramebuffer> framebuffers;
	ImGui_ImplVulkanH_Window vw;

};

