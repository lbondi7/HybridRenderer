#include "RasterRenderer.h"

#include "DebugLogger.h"
#include "ImGUI_.h"

RasterRenderer::RasterRenderer(Window* window, VulkanCore* core, SwapChain* swapChain)
{
    swapChain = nullptr;
    deviceContext = nullptr;
    resources = nullptr;
}

void RasterRenderer::Initialise(Window* window, VulkanCore* core, SwapChain* swapChain, Resources* _resources)
{
    deviceContext = core->deviceContext.get();
    this->swapChain = swapChain;
    resources = _resources;

    imgui.create(window->glfwWindow, core->instance, core->surface, deviceContext, swapChain);
    
    RenderPassInfo info{};

    info.attachments.push_back({ AttachmentType::COLOUR, swapChain->imageFormat, VK_ATTACHMENT_LOAD_OP_CLEAR,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED });
    info.attachments.push_back({ AttachmentType::DEPTH, Utility::findDepthFormat(deviceContext->physicalDevice), VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED });


    info.dependencies.emplace_back(Initialisers::subpassDependency(VK_SUBPASS_EXTERNAL, 0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));
    renderPass.Create(deviceContext, info);
    info.attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    penultimateRenderPass.Create(deviceContext, info);

    PipelineInfo pipelineInfo{};
    shadowMap.Create(deviceContext, 3);
    pipelineInfo.shaders = { resources->GetShader("shadowmapping/offscreen", VK_SHADER_STAGE_VERTEX_BIT) ,
        resources->GetShader("shadowmapping/offscreen", VK_SHADER_STAGE_FRAGMENT_BIT) };
    pipelineInfo.vertexInputAttributes = Vertex::getAttributeDescriptions({ VertexAttributes::POSITION , VertexAttributes::UV_COORD});
    pipelineInfo.vertexInputBindings = { Vertex::getBindingDescription() };
    pipelineInfo.depthBiasEnable = VK_TRUE;
    pipelineInfo.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS };
    pipelineInfo.conservativeRasterisation = true;
    pipelineInfo.colourAttachmentCount = 0;
    pipelineInfo.layoutsName = "offscreen";
    pipelineInfo.cullMode = VK_CULL_MODE_FRONT_BIT;

    shadowMap.Initialise(pipelineInfo);

    pipelineInfo.vertexInputAttributes = Vertex::getAttributeDescriptions({ VertexAttributes::POSITION, VertexAttributes::UV_COORD, VertexAttributes::V_COLOUR, VertexAttributes::NORMAL });
    pipelineInfo.shaders = { resources->GetShader("shadowmapping/scene", VK_SHADER_STAGE_VERTEX_BIT) ,
        resources->GetShader("shadowmapping/scene", VK_SHADER_STAGE_FRAGMENT_BIT) };
    pipelineInfo.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    pipelineInfo.layoutsName = "scene";
    pipelineInfo.colourAttachmentCount = 1;
    pipelineInfo.conservativeRasterisation = false;
    pipelineInfo.depthBiasEnable = VK_FALSE;
    pipelineInfo.cullMode = VK_CULL_MODE_BACK_BIT;

    pipeline.Create(deviceContext, &renderPass, pipelineInfo);

    frameBuffer.Create(deviceContext, renderPass.vkRenderPass);
    penultimateFrameBuffer.Create(deviceContext, penultimateRenderPass.vkRenderPass);
    for (size_t i = 0; i < swapChain->imageCount; i++)
    {
        std::vector<VkImageView> attachments{ swapChain->images[i].imageView, swapChain->depthImage.imageView };
        frameBuffer.createFramebuffer(attachments, swapChain->extent);
        penultimateFrameBuffer.createFramebuffer(attachments, swapChain->extent);
    }

   // resources->GetModel("tree2");


    AllocateCommandBuffers();

    commandBuffersReady = false;
}

void RasterRenderer::Deinitialise(bool total) {

    vkDeviceWaitIdle(deviceContext->logicalDevice);

    frameBuffer.Destroy();
    penultimateFrameBuffer.Destroy();

    pipeline.Destroy(false);
    renderPass.Destroy();
    penultimateRenderPass.Destroy();

    imgui.deinit();


    if (total) {
        imgui.destroy();

        vkFreeCommandBuffers(deviceContext->logicalDevice, deviceContext->commandPool,
            static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

        shadowMap.Destroy(true);
    }
}

void RasterRenderer::Reinitialise() {

    Deinitialise();

    renderPass.Init();
    penultimateRenderPass.Init();

    frameBuffer.Create(deviceContext, renderPass.vkRenderPass);
    penultimateFrameBuffer.Create(deviceContext, penultimateRenderPass.vkRenderPass);
    VkExtent2D _extent = { swapChain->extent.width, swapChain->extent.height};
    for (size_t i = 0; i < swapChain->imageCount; i++)
    {
        std::vector<VkImageView> attachments{ swapChain->images[i].imageView, swapChain->depthImage.imageView };
        frameBuffer.createFramebuffer(attachments, _extent);
        penultimateFrameBuffer.createFramebuffer(attachments, _extent);
    }

    shadowMap.Reinitialise(true);
    pipeline.Init();
    imgui.reinit();

    commandBuffersReady = false;

}

void RasterRenderer::AllocateCommandBuffers() {
    commandBuffers.resize(frameBuffer.frames.size());

    VkCommandBufferAllocateInfo allocInfo = Initialisers::commandBufferAllocateInfo(deviceContext->commandPool, static_cast<uint32_t>(commandBuffers.size()));

    if (vkAllocateCommandBuffers(deviceContext->logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");

}

void RasterRenderer::buildCommandBuffers(Camera* camera, Scene* scene)
{
    vkQueueWaitIdle(deviceContext->presentQueue);
    for (int32_t i = 0; i < commandBuffers.size(); ++i)
    {
        rebuildCommandBuffer(i, camera, scene);
    }
    commandBuffersReady = true;
}

void RasterRenderer::rebuildCommandBuffer(uint32_t i, Camera* camera, Scene* scene) {

    VkCommandBufferBeginInfo beginInfo = Initialisers::commandBufferBeginInfo();

    std::array<VkClearValue, 2> clearValues;

    if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    {
        clearValues[0].depthStencil = { 1.0f, 0 };

        auto& frame = shadowMap.frameBuffer.frames[i];
        shadowMap.renderPass.Begin(commandBuffers[i], frame.vkFrameBuffer, frame.extent, &clearValues[0]);

        vkCmdSetDepthBias(commandBuffers[i], depthBias.x, depthBias.y, depthBias.z);

        VkViewport viewport = Initialisers::viewport(0, 0, (float)frame.extent.width, (float)frame.extent.height);
        VkRect2D scissor = Initialisers::scissor(frame.extent);

        vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipeline.vkPipeline);

        std::vector<VkDescriptorSet> descriptorSets;
        descriptorSets.resize(2);
        descriptorSets[0] = scene->lightDescriptor.sets[i];
        for (auto& go : scene->gameObjects) {
            if (!go.shadowCaster || !go.mesh)
                continue;

            descriptorSets[1] = go.descriptor.sets[i];

            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipeline.pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

            go.mesh->Bind(commandBuffers[i]);

            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(go.mesh->indices.size()), 1, 0, 0, 0);  
        }

        shadowMap.renderPass.End(commandBuffers[i]);
    }

    {
        clearValues[0].color = { 0.25f, 0.25f, 0.25f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };
        if(!ImGUI::enabled)
            renderPass.Begin(commandBuffers[i], frameBuffer.frames[i].vkFrameBuffer, frameBuffer.frames[i].extent, clearValues.data(), static_cast<uint32_t>(clearValues.size()));
        else
            penultimateRenderPass.Begin(commandBuffers[i], penultimateFrameBuffer.frames[i].vkFrameBuffer, penultimateFrameBuffer.frames[i].extent, clearValues.data(), static_cast<uint32_t>(clearValues.size()));

        vkCmdSetDepthBias(commandBuffers[i], 0.0f, 0.0f, 0.0f);
        camera->vkSetViewport(commandBuffers[i]);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vkPipeline);
        std::array<VkDescriptorSet, 5> descriptorSets = { camera->descriptor.sets[i], VK_NULL_HANDLE, scene->lightDescriptor.sets[i], shadowMap.descriptor.sets[i], scene->asDescriptor.sets[i]};
        //std::array<VkDescriptorSet, 4> descriptorSets = { camera->descriptor.sets[i], VK_NULL_HANDLE, scene->lightDescriptor.sets[i], shadowMap.descriptor.sets[i] };

        for (auto& go : scene->gameObjects) {
            if (!go.mesh)
                continue;

            descriptorSets[1] = go.descriptor.sets[i];

            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

            go.mesh->Bind(commandBuffers[i]);

            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(go.mesh->indices.size()), 1, 0, 0, 0);
        }

        if (!ImGUI::enabled)
            renderPass.End(commandBuffers[i]);
        else
            penultimateRenderPass.End(commandBuffers[i]);
    }

    if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void RasterRenderer::Prepare()
{
    if (ImGUI::enabled && !imgui.startedFrame)
    {
        imgui.startFrame();

        if (widget.enabled) {

            if (widget.NewWindow("Raster Render")) {

                if (widget.Slider3("Depth Bias", depthBias, -5.0, 5.0)) 
                {
                    commandBuffersReady = false;
                }

                //widget.Image(0, { swapChain->extent.width / storageImageSize, swapChain->extent.height / storageImageSize });
            }
            widget.EndWindow();
        }
    }
}

void RasterRenderer::GetCommandBuffer(uint32_t imageIndex, std::vector<VkCommandBuffer>& submitCommandBuffers, Camera* camera, Scene* scene)
{
    if (shadowMap.Update(imageIndex) || !commandBuffersReady)
    {
        buildCommandBuffers(camera, scene);
    }

    submitCommandBuffers.emplace_back(commandBuffers[imageIndex]);
}

void RasterRenderer::GetImGuiCommandBuffer(uint32_t imageIndex, std::vector<VkCommandBuffer>& submitCommandBuffers, VkExtent2D extent)
{
    if (imgui.startedFrame) {

        imgui.endFrame();
        imgui.drawn = true;

        imgui.buildCommandBuffer(imageIndex, extent);
        submitCommandBuffers.emplace_back(imgui.commandBuffers[imageIndex]);
    }
}