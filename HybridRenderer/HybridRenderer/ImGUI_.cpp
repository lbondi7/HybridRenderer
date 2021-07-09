#include "ImGUI_.h"

#include "Initilizers.h"
#include "Utility.h"

#include <array>

ImGUI::~ImGUI()
{
	devices = nullptr;
}

void ImGUI::createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags) {
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = devices->indices.graphicsFamily.value();
	commandPoolCreateInfo.flags = flags;

	if (vkCreateCommandPool(devices->logicalDevice, &commandPoolCreateInfo, nullptr, commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Could not create graphics command pool");
	}
}

void ImGUI::createCommandBuffers(VkCommandBuffer* commandBuffer, uint32_t commandBufferCount, VkCommandPool& commandPool) {
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = commandBufferCount;
	vkAllocateCommandBuffers(devices->logicalDevice, &commandBufferAllocateInfo, commandBuffer);
}


void ImGUI::create(GLFWwindow * window, VkInstance instance, VkSurfaceKHR surface, DeviceContext* _devices, SwapChain* _swapChain)
{

	devices = _devices;
	swapChain = _swapChain;

	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	auto err = vkCreateDescriptorPool(devices->logicalDevice, &pool_info, nullptr, &descriptorPool);

	RenderPassInfo info{};
	info.attachments.push_back({ AttachmentType::COLOUR, swapChain->imageFormat, VK_ATTACHMENT_LOAD_OP_LOAD, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	info.dependencies.emplace_back(Initialisers::subpassDependency(VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT));

	renderPass.Create(devices, info);

	reinit();

	QueueFamilyIndices queueFamilyIndices = devices->indices;// findQueueFamilies(physicalDevice);
	imGuiCommandPools.resize(swapChain->imageCount);
	commandBuffers.resize(swapChain->imageCount);

	createCommandPool(&commandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	createCommandBuffers(commandBuffers.data(), static_cast<uint32_t>(commandBuffers.size()), commandPool);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = devices->physicalDevice;
	init_info.Device = devices->logicalDevice;
	init_info.QueueFamily = 0;
	init_info.Queue = devices->graphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = descriptorPool;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = swapChain->imageCount;
	init_info.ImageCount = swapChain->imageCount;
	//init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, renderPass.vkRenderPass);

	auto cmdBuffer = devices->generateCommandBuffer();
	ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
	devices->EndCommandBuffer(cmdBuffer);


}


void ImGUI::Render() {

}


void ImGUI::reinit()
{

	frameBuffer.Create(devices, renderPass.vkRenderPass);
	for (size_t i = 0; i < swapChain->imageCount; i++)
	{
		std::vector<VkImageView> attachments{ swapChain->images[i].imageView };
		frameBuffer.createFramebuffer(attachments, swapChain->extent);
	}


}


void ImGUI::startFrame() {
	ImGui::NewFrame();

}

void ImGUI::NewWindow(const char* name, bool* close) {

	float x = 0.0f;
	float xarr[2]{ 0.0f, 0.0f };
	float yarr[3]{ 0.0f, 0.0f, 0.0f };
	float zarr[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
	bool boo = true;
	bool enabled = true;

	if (ImGui::BeginMainMenuBar()) {
		ImGui::MenuItem("Main Menu Item 1", "MMI1");
		ImGui::MenuItem("Main Menu Item 2", "MMI2");
		ImGui::MenuItem("Main Menu Item 3", "MMI3");
	}
	ImGui::EndMainMenuBar();

	if (ImGui::Begin(name, close)) {
		renderStuff = true;

		if (ImGui::BeginMenuBar()) {
			ImGui::MenuItem("Menu Item 1", "MI1");
			ImGui::MenuItem("Menu Item 2", "MI2");
			ImGui::MenuItem("Menu Item 3", "MI3");
			ImGui::EndMenuBar();
		}

		ImGui::Text("Text");
		ImGui::Checkbox("Check Box", &boo);
		ImGui::RadioButton("Radio Button", true);
		ImGui::Button("Button", { 10, 10 });
		ImGui::ArrowButton("Arrow Button Right", ImGuiDir_::ImGuiDir_Right);
		ImGui::ArrowButton("Arrow Button Up", ImGuiDir_::ImGuiDir_Up);
		if (ImGui::BeginCombo("Combo", "Preview Combo", ImGuiComboFlags_::ImGuiComboFlags_HeightLarge))
		{
			ImGui::TextColored({ 0, 1, 0, 1 }, "Text Coloured");
			ImGui::Text("Combo Text");
			ImGui::SliderFloat("Slider Float", &x, -1.0f, 1.0f, "none", 1.0f);
			ImGui::SliderFloat2("Slider Float", xarr, -1.0f, 1.0f, "none", 1.0f);
			ImGui::SliderFloat3("Slider Float", yarr, -1.0f, 1.0f, "none", 1.0f);
			ImGui::SliderFloat4("Slider Float", zarr, -1.0f, 1.0f, "none", 1.0f);
			ImGui::EndCombo();
		}
	}
	ImGui::End();

}

void ImGUI::endFrame() {

	// Render to generate draw buffers

	ImGui::Render();

	renderStuff = false;
}

void ImGUI::deinit()
{
	frameBuffer.Destroy();
}

void ImGUI::destroy(bool complete)
{

	renderPass.Destroy();

	vkFreeCommandBuffers(devices->logicalDevice, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vkDestroyCommandPool(devices->logicalDevice , commandPool, nullptr);

	// Resources to destroy when the program ends
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	vkDestroyDescriptorPool(devices->logicalDevice, descriptorPool, nullptr);
}

void ImGUI::update(VkExtent2D extent)
{
	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2(static_cast<float>(extent.width), static_cast<float>(extent.height));
	io.MouseWheel = mouseWheel;

	io.MousePos = ImVec2(mousePos.x, mousePos.y);
	io.MouseDown[0] = leftMouse;
	io.MouseDown[1] = rightMouse;

	//newFrame();
}

std::vector<VkVertexInputBindingDescription> ImGUI::bindingDescriptions()
{
	return {
	Initialisers::vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
	};
}

std::vector<VkVertexInputAttributeDescription> ImGUI::attributeDescriptions()
{
	return  {
		Initialisers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
		Initialisers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	// Location 1: UV
		Initialisers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
	};
}
