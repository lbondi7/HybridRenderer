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

	//if (ImGui::Begin("Depth Texture"));
	//ImGui::Image((ImTextureID)ImGui_ImplVulkan_AddTexture(sampler, view, layout), ImVec2(250, 250));
	//ImGui::End();
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
