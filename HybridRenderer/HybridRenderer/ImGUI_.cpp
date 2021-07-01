#include "ImGUI_.h"

#include "Initilizers.h"
#include "Utility.h"

#include <array>

ImGUI::~ImGUI()
{
	devices = nullptr;
}

void ImGUI::create(DeviceContext* _devices, RenderPass* _renderPass, DescriptorSetManager* _descriptorSetManager, const PipelineInfo& _pipelineInfo)
{
	devices = _devices;
	pipelineInfo = _pipelineInfo;
	dsm = _descriptorSetManager;
	dpipeline.Create(devices, _renderPass, dsm, _pipelineInfo);

	init();

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


void ImGUI::create(GLFWwindow * window, VkInstance instance, VkSurfaceKHR surface, DeviceContext* _devices, SwapChain* swapChain, RenderPass* _renderPass)
{
	devices = _devices;


	vw.Surface = surface;

	// Check for WSI support
	VkBool32 res;
	vkGetPhysicalDeviceSurfaceSupportKHR(devices->physicalDevice, devices->indices.graphicsFamily.value(), vw.Surface, &res);
	if (res != VK_TRUE)
	{
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	// Select Surface Format
	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	vw.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(devices->physicalDevice, vw.Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

	// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR };
#endif
	vw.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(devices->physicalDevice, vw.Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
	//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

	// Create SwapChain, RenderPass, Framebuffer, etc.
	ImGui_ImplVulkanH_CreateWindow(instance, devices->physicalDevice, devices->logicalDevice, &vw, devices->indices.graphicsFamily.value(), nullptr, 800, 600, swapChain->imageCount);


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	std::vector<VkDescriptorPoolSize> poolSizes{
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

	VkDescriptorPoolCreateInfo poolInfo = Initialisers::descriptorPoolCreateInfo(static_cast<uint32_t>(poolSizes.size()), poolSizes.data(), 1000);

	if (vkCreateDescriptorPool(devices->logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}


	VkAttachmentDescription attachment = {};
	attachment.format = swapChain->imageFormat;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


	VkAttachmentReference color_attachment = {};
	color_attachment.attachment = 0;
	color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment;


	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &attachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = 1;
	info.pDependencies = &dependency;
	if (vkCreateRenderPass(devices->logicalDevice, &info, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Could not create Dear ImGui's render pass");
	}

	VkImageView attachment2[1];
	VkFramebufferCreateInfo Framebuffeinfo = {};
	Framebuffeinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	Framebuffeinfo.renderPass = renderPass;
	Framebuffeinfo.attachmentCount = 1;
	Framebuffeinfo.pAttachments = attachment2;
	Framebuffeinfo.width = WIDTH;
	Framebuffeinfo.height = HEIGHT;
	Framebuffeinfo.layers = 1;
	framebuffers.resize(3);
	for (uint32_t i = 0; i < swapChain->imageCount; i++)
	{
		attachment2[0] = swapChain->images[i].imageView;
		vkCreateFramebuffer(devices->logicalDevice, &Framebuffeinfo, nullptr, &framebuffers[i]);
	}


	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = devices->physicalDevice;
	init_info.Device = devices->logicalDevice;
	init_info.QueueFamily = devices->indices.graphicsFamily.value();
	init_info.Queue = devices->graphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = descriptorPool;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info, renderPass);



	VkCommandBuffer command_buffer = devices->generateCommandBuffer();
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
	devices->EndCommandBuffer(command_buffer);
	//ImGui_ImplVulkan_DestroyFontUploadObjects();




	//VkCommandBuffer command_buffer = devices->generateCommandBuffer();
	//ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
	//devices->EndCommandBuffer(command_buffer);


	createCommandPool(&commandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	commandBuffers.resize(swapChain->imageCount);
	createCommandBuffers(commandBuffers.data(), static_cast<uint32_t>(commandBuffers.size()), commandPool);


}


void ImGUI::Render() {

}


void ImGUI::init()
{
	ImGui::CreateContext();

	//pushConstBlock = {};
	/// init

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	// Dimensions
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(WIDTH, HEIGHT);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);


	/// init resources

	// Create font texture
	unsigned char* fontData;
	int texWidth, texHeight;
	io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
	VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);


	fontImage.Create(devices, texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM);
	fontImage.createImageView(VK_IMAGE_ASPECT_COLOR_BIT);

	// Staging buffers for font data upload
	Buffer stagingBuffer;
	stagingBuffer.Create(devices, uploadSize);

	stagingBuffer.Map(fontData);

	fontImage.transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	fontImage.CopyFromBuffer(stagingBuffer.vkBuffer);

	fontImage.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	stagingBuffer.Destroy();

	VkSamplerCreateInfo samplerInfo = Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, 1.0f, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

	fontImage.createSampler(samplerInfo);

	// Descriptor pool
	//std::vector<VkDescriptorPoolSize> poolSizes = {
	//	Initialisers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
	//};
	//VkDescriptorPoolCreateInfo descriptorPoolInfo = Initialisers::descriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), 2);
	//vkCreateDescriptorPool(devices->logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool);

	//// Descriptor set layout
	//std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
	//	Initialisers::descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
	//};
	//VkDescriptorSetLayoutCreateInfo descriptorLayout = Initialisers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), 1);
	//vkCreateDescriptorSetLayout(devices->logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout);

	//// Descriptor set
	//VkDescriptorSetAllocateInfo allocInfo = Initialisers::descriptorSetAllocateInfo(descriptorPool, 1, &descriptorSetLayout);
	//vkAllocateDescriptorSets(devices->logicalDevice, &allocInfo, &descriptorSet);
	//VkDescriptorImageInfo fontDescriptor = Initialisers::descriptorImageInfo(
	//	sampler,
	//	fontView,
	//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	//);

	DescriptorSetRequest request{};
	request.requests.push_back({ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });
	dsm->getDescriptorSets(descriptorSets, request);

	for (size_t i = 0; i < 3; i++)
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
	Initialisers::writeDescriptorSet(descriptorSets[i], 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &fontImage.descriptorInfo)
		};
		vkUpdateDescriptorSets(devices->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}

	//std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
	//	Initialisers::writeDescriptorSet(descriptorSet, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &fontImage.descriptorInfo)
	//};
	//vkUpdateDescriptorSets(devices->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

	//// Pipeline cache
	//VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	//pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	//vkCreatePipelineCache(devices->logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache);

	//// Pipeline layout
	//// Push constants for UI rendering parameters
	//VkPushConstantRange pushConstantRange{}; // = Initialisers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock), 0);
	//pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	//pushConstantRange.size = sizeof(PushConstBlock);
	//pushConstantRange.offset = 0;

	//VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Initialisers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
	//pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	//pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	//vkCreatePipelineLayout(devices->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

	//// Setup graphics pipeline for UI rendering
	//VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
	//	Initialisers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

	//VkPipelineRasterizationStateCreateInfo rasterizationState =
	//	Initialisers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, 1.0f, VK_FALSE);

	//// Enable blending
	//VkPipelineColorBlendAttachmentState blendAttachmentState{};
	//blendAttachmentState.blendEnable = VK_TRUE;
	//blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	//blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	//blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	//blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	//VkPipelineColorBlendStateCreateInfo colorBlendState =
	//	Initialisers::pipelineColourBlendStateCreateInfo(&blendAttachmentState, 1);

	//VkPipelineDepthStencilStateCreateInfo depthStencilState =
	//	Initialisers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

	//VkPipelineViewportStateCreateInfo viewportState =
	//	Initialisers::pipelineViewportStateCreateInfo(nullptr, 1, nullptr, 1);

	//VkPipelineMultisampleStateCreateInfo multisampleState =
	//	Initialisers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

	//std::vector<VkDynamicState> dynamicStateEnables = {
	//	VK_DYNAMIC_STATE_VIEWPORT,
	//	VK_DYNAMIC_STATE_SCISSOR
	//};
	//VkPipelineDynamicStateCreateInfo dynamicState =
	//	Initialisers::pipelineDynamicStateCreateInfo(dynamicStateEnables.data() , 2);

	//std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

	//VkGraphicsPipelineCreateInfo pipelineCreateInfo = Initialisers::graphicsPipelineCreateInfo(pipelineLayout, renderPass, 0, pipeline);

	//pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	//pipelineCreateInfo.pRasterizationState = &rasterizationState;
	//pipelineCreateInfo.pColorBlendState = &colorBlendState;
	//pipelineCreateInfo.pMultisampleState = &multisampleState;
	//pipelineCreateInfo.pViewportState = &viewportState;
	//pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	//pipelineCreateInfo.pDynamicState = &dynamicState;
	//pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	//pipelineCreateInfo.pStages = shaderStages.data();

	//// Vertex bindings an attributes based on ImGui vertex definition
	//std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
	//	Initialisers::vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
	//};
	//std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
	//	Initialisers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
	//	Initialisers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	// Location 1: UV
	//	Initialisers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
	//};
	//VkPipelineVertexInputStateCreateInfo vertexInputState = Initialisers::pipelineVertexInputStateCreateInfo();
	//vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
	//vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
	//vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
	//vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

	//pipelineCreateInfo.pVertexInputState = &vertexInputState;

	//shaderStages[0] = pipelineInfo.shaders[0]->createInfo();
	//shaderStages[1] = pipelineInfo.shaders[1]->createInfo();
	//
	//vkCreateGraphicsPipelines(devices->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline);



	// update Buffers

	//newFrame();
	//updateBuffers();

}

void ImGUI::reinit()
{
	dpipeline.Init();
}

void ImGUI::Draw(VkCommandBuffer commandBuffer, size_t cmdBufferIndex) {


	ImGuiIO& io = ImGui::GetIO();
	ImDrawData* imDrawData = ImGui::GetDrawData();

	if (!imDrawData)
		return;

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dpipeline.pipelineLayout, 0, 1, &descriptorSets[cmdBufferIndex], 0, nullptr);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dpipeline.vkPipeline);

	VkViewport viewport = Initialisers::viewport(0.0f, 0.0f, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	// UI scale and translate via push constants
	pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
	pushConstBlock.translate = glm::vec2(-1.0f);
	vkCmdPushConstants(commandBuffer, dpipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

	// Render commands
	int32_t vertexOffset = 0;
	int32_t indexOffset = 0;

	if (imDrawData->CmdListsCount > 0) {

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.vkBuffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT16);

		for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[i];
			for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
				VkRect2D scissorRect;
				scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
				scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
				scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
				vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}
	}
}

void ImGUI::updateBuffers()
{

	ImDrawData* imDrawData = ImGui::GetDrawData();

	if (!imDrawData)
		return;


	// Note: Alignment is done inside buffer creation
	VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

	if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
		return;
	}

	// Update buffers only if vertex or index count has been changed compared to current buffer size

	// Vertex buffer
	if ((vertexBuffer.vkBuffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
		vertexBuffer.Unmap();
		vertexBuffer.Destroy();
		vertexBuffer.Create(devices, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		vertexCount = imDrawData->TotalVtxCount;
		vertexBuffer.Map();
	}

	// Index buffer
	if ((indexBuffer.vkBuffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount)) {
		indexBuffer.Unmap();
		indexBuffer.Destroy();
		indexBuffer.Create(devices, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		indexCount = imDrawData->TotalIdxCount;
		indexBuffer.Map();
	}

	// Upload data
	ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer.data;
	ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer.data;

	
	for (int n = 0; n < imDrawData->CmdListsCount; n++) {
		const ImDrawList* cmd_list = imDrawData->CmdLists[n];
		memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtxDst += cmd_list->VtxBuffer.Size;
		idxDst += cmd_list->IdxBuffer.Size;
	}

	// Flush to make writes visible to GPU
	vertexBuffer.Flush();
	indexBuffer.Flush();
}

void ImGUI::newFrame() {
	//ImGui::NewFrame();

	//// Init imGui windows and elements

	//ImGui::ShowDemoWindow();

	//// Render to generate draw buffers
	//ImGui::Render();
}

void ImGUI::startFrame() {
	ImGui::NewFrame();

}

void ImGUI::NewWindow(const char* name, bool * close) {

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

void ImGUI::NewImage(VkSampler sampler, VkImageView view, VkImageLayout layout) {

	if (ImGui::Begin("Depth Texture"));
	ImGui::Image((ImTextureID)ImGui_ImplVulkan_AddTexture(sampler, view, layout), ImVec2(250, 250));
	ImGui::End();
}

void ImGUI::endFrame() {

	// Render to generate draw buffers

	ImGui::Render();

	renderStuff = false;
}

void ImGUI::deinit()
{

	dpipeline.Destroy(false);
	//vkDestroyDescriptorPool(devices->logicalDevice, descriptorPool, nullptr);
	//vkDestroyPipeline(devices->logicalDevice, pipeline, nullptr);
	//vkDestroyPipelineCache(devices->logicalDevice, pipelineCache, nullptr);
}

void ImGUI::destroy(bool complete)
{

	//for (auto framebuffer : framebuffers) {
	//	vkDestroyFramebuffer(devices->logicalDevice, framebuffer, nullptr);
	//}

	//vkDestroyRenderPass(devices->logicalDevice, renderPass, nullptr);

	//vkFreeCommandBuffers(devices->logicalDevice, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	//vkDestroyCommandPool(devices->logicalDevice, commandPool, nullptr);

	//// Resources to destroy when the program ends
	//ImGui_ImplVulkan_Shutdown();
	//ImGui_ImplGlfw_Shutdown();
	//ImGui::DestroyContext();
	//vkDestroyDescriptorPool(devices->logicalDevice, descriptorPool, nullptr);

	fontImage.Destroy();
	vertexBuffer.Destroy();
	indexBuffer.Destroy();
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
