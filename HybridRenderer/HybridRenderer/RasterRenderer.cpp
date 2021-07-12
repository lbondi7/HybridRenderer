#include "RasterRenderer.h"

#include "DebugLogger.h"
#include "ImGUI_.h"

RasterRenderer::RasterRenderer(Window* window, VulkanCore* core)
{
    deviceContext = core->deviceContext.get();

    swapChain.Create(core->surface, deviceContext, &window->width, &window->height);

    imgui.create(window->glfwWindow, core->instance, core->surface, deviceContext, &swapChain);
}

void RasterRenderer::run() {

    //Log("How many Objects? ");
    //std::cin >> gameObjectCount;
    //mainLoop();
    //cleanup();
}

void RasterRenderer::initialise(Resources* _resources, DescriptorSetManager* _descriptorSetManager)
{

    resources = _resources;
    descriptorSetManager = _descriptorSetManager;
      
    //swapChain.Create(window.glfwWindow, surface, deviceContext);

    RenderPassInfo info{};
    info.attachments.push_back({ AttachmentType::COLOUR, swapChain.imageFormat, VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED});
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

    pipelineInfo.shaders = { resources->vertexShaders["scene"].get(), resources->fragmentShaders["scene"].get() };
    pipelineInfo.vertexInputAttributes = Vertex::getAttributeDescriptions({ VertexAttributes::POSITION, VertexAttributes::UV_COORD, VertexAttributes::V_COLOUR, VertexAttributes::NORMAL });
    pipelineInfo.vertexInputBindings = { Vertex::getBindingDescription() };
    pipelineInfo.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    //pipelineInfo.polygonMode = VK_POLYGON_MODE_LINE;

    pipeline.Create(deviceContext, &renderPass, pipelineInfo);

    frameBuffer.Create(deviceContext, renderPass.vkRenderPass);
    penultimateFrameBuffer.Create(deviceContext, penultimateRenderPass.vkRenderPass);
    for (size_t i = 0; i < swapChain.imageCount; i++)
    {
        std::vector<VkImageView> attachments{ swapChain.images[i].imageView, swapChain.depthImage.imageView };
        frameBuffer.createFramebuffer(attachments, swapChain.extent);
        penultimateFrameBuffer.createFramebuffer(attachments, swapChain.extent);
    }

    shadowMap.Create(deviceContext, &swapChain);
    pipelineInfo.shaders = { resources->vertexShaders["offscreen"].get() };
    pipelineInfo.depthBiasEnable = VK_TRUE;
    pipelineInfo.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS };
    pipelineInfo.vertexInputAttributes = Vertex::getAttributeDescriptions({ VertexAttributes::POSITION});
    pipelineInfo.conservativeRasterisation = true;
    pipelineInfo.collorAttachmentCount = 0;

    shadowMap.Init(pipelineInfo);

    AllocateCommandBuffers();
    createSyncObjects();
    auto& texture = shadowMap.depthTexture;

    //descTest = ImGui_ImplVulkan_AddTextureD(texture.sampler, texture.imageView, texture.descriptorInfo.imageLayout);

    commandBuffersReady = false;
}

void RasterRenderer::cleanupSwapChain() {

    vkDeviceWaitIdle(deviceContext->logicalDevice);

    frameBuffer.Destroy();
    penultimateFrameBuffer.Destroy();

    vkFreeCommandBuffers(deviceContext->logicalDevice, deviceContext->commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    pipeline.Destroy(false);
    renderPass.Destroy();
    penultimateRenderPass.Destroy();

    swapChain.Destroy();


    //imgui.destroy();
    imgui.deinit();

}

void RasterRenderer::cleanup() {
    cleanupSwapChain();

    //for (size_t i = 0; i < swapChain.imageCount; i++) {
    //    lightBuffers[i].Destroy();
    //    cameraBuffers[i].Destroy();
    //}

    imgui.destroy();

    shadowMap.Destroy(true);


    //resources.Destroy();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(deviceContext->logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(deviceContext->logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(deviceContext->logicalDevice, inFlightFences[i], nullptr);
    }

    //vkDestroyCommandPool(deviceContext->logicalDevice, deviceContext->commandPool, nullptr);

    //deviceContext->Destroy();

    //if (enableValidationLayers) {
    //    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    //}

    //vkDestroySurfaceKHR(instance, surface, nullptr);
    //vkDestroyInstance(instance, nullptr);

    //window.destroy();
}

void RasterRenderer::recreateSwapChain() {

    //window.resize();
    //vkDeviceWaitIdle(deviceContext->logicalDevice);

    cleanupSwapChain();

    swapChain.Init();
    renderPass.Init();
    penultimateRenderPass.Init();

    frameBuffer.Create(deviceContext, renderPass.vkRenderPass);
    penultimateFrameBuffer.Create(deviceContext, penultimateRenderPass.vkRenderPass);
    for (size_t i = 0; i < swapChain.imageCount; i++)
    {
        std::vector<VkImageView> attachments{ swapChain.images[i].imageView, swapChain.depthImage.imageView };
        frameBuffer.createFramebuffer(attachments, swapChain.extent);
        penultimateFrameBuffer.createFramebuffer(attachments, swapChain.extent);
    }

    shadowMap.reinit(true);
    pipeline.Init();
    //camera.init(swapChain.extent);
    imgui.reinit();
    AllocateCommandBuffers();

    imagesInFlight.resize(swapChain.imageCount, VK_NULL_HANDLE);

    auto& texture = shadowMap.depthTexture;

   //descTest = ImGui_ImplVulkan_AddTextureD(texture.sampler, texture.imageView, texture.descriptorInfo.imageLayout);

    commandBuffersReady = false;

}

void RasterRenderer::AllocateCommandBuffers() {
    commandBuffers.resize(frameBuffer.frames.size());

    VkCommandBufferAllocateInfo allocInfo = Initialisers::commandBufferAllocateInfo(deviceContext->commandPool, static_cast<uint32_t>(commandBuffers.size()));

    if (vkAllocateCommandBuffers(deviceContext->logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");

}

void RasterRenderer::buildCommandBuffers(Camera* camera, std::vector<GameObject>& gameObjects, Descriptor& lightDescs) 
{
    vkQueueWaitIdle(deviceContext->presentQueue);
    for (int32_t i = 0; i < commandBuffers.size(); ++i)
    {
        rebuildCommandBuffer(i, camera, gameObjects, lightDescs);
    }
    commandBuffersReady = true;
}

void RasterRenderer::rebuildCommandBuffer(uint32_t i, Camera* camera, std::vector<GameObject>& gameObjects, Descriptor& lightDescs) {

    VkCommandBufferBeginInfo beginInfo = Initialisers::commandBufferBeginInfo();

    std::array<VkClearValue, 2> clearValues;

    //if(imgui.enabled)
    //    imgui.updateBuffers();

    if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    {
        clearValues[0].depthStencil = { 1.0f, 0 };

        auto& frame = shadowMap.frameBuffer.frames[i];
        shadowMap.renderPass.Begin(commandBuffers[i], frame.vkFrameBuffer, frame.extent, &clearValues[0]);

        // Set depth bias (aka "Polygon offset")
        // Required to avoid shadow mapping artifacts
        vkCmdSetDepthBias(commandBuffers[i], 1.25f, 0.0f, 1.75f);

        VkViewport viewport = Initialisers::viewport(0, 0, (float)frame.extent.width, (float)frame.extent.height);
        VkRect2D scissor = Initialisers::scissor(frame.extent);

        vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipeline.vkPipeline);

        int j = 0;
        for (auto& go : gameObjects) {
            if (!go.shadowCaster)
                continue;

            std::array<VkDescriptorSet, 2> descriptorSets = { lightDescs.sets[i] , go.offscreenDescriptor.sets[i] };
            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipeline.pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
            go.mesh->Bind(commandBuffers[i]);
            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(go.mesh->indices.size()), 1, 0, 0, 0);
            j++;
        }

        shadowMap.renderPass.End(commandBuffers[i]);
    }

    {
        clearValues[0].color = { 0.025f, 0.025f, 0.025f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };

        if(!ImGUI::enabled)
            renderPass.Begin(commandBuffers[i], frameBuffer.frames[i].vkFrameBuffer, swapChain.extent, clearValues.data(), static_cast<uint32_t>(clearValues.size()));
        else
            penultimateRenderPass.Begin(commandBuffers[i], penultimateFrameBuffer.frames[i].vkFrameBuffer, swapChain.extent, clearValues.data(), static_cast<uint32_t>(clearValues.size()));

        camera->vkSetViewport(commandBuffers[i]);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vkPipeline);

        for (auto& go : gameObjects) {

            std::array<VkDescriptorSet, 4> descriptorSets = { camera->descriptor.sets[i], go.descriptor.sets[i], lightDescs.sets[i], shadowMap.descriptor.sets[i] };

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

    if (ImGUI::enabled) {
        imgui.buildCommandBuffers(i, swapChain.extent);
    }
}

void RasterRenderer::createSyncObjects() {

    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChain.imageCount, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = Initialisers::semaphoreCreateInfo();

    VkFenceCreateInfo fenceInfo = Initialisers::fenceCreateInfo();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(deviceContext->logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(deviceContext->logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(deviceContext->logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

//void RasterRenderer::createCamera() {
//    camera.lookAt = glm::vec3(0, 0, 0);
//    camera.transform.position = glm::vec3(0, 0, 10);
//    camera.transform.rotation.y = 180.f;
//
//    camera.init(swapChain.extent);
//}

void RasterRenderer::prepare()
{
    VkSemaphore iAS = imageAvailableSemaphores[currentFrame];

    VkResult result = vkAcquireNextImageKHR(deviceContext->logicalDevice, swapChain.vkSwapChain, UINT64_MAX, iAS, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
    return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (ImGUI::enabled && !imgui.startedFrame)
    {
        imgui.startFrame();
    }


}


void RasterRenderer::render(Camera* camera, std::vector<GameObject>& gameObjects, Descriptor& lightDescs)
{

    shadowMap.update();

    if (!ImGUI::enabled && !imgui.startedFrame &&  !commandBuffersReady)
    {
        buildCommandBuffers(camera, gameObjects, lightDescs);
    }
    

    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    vkWaitForFences(deviceContext->logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(deviceContext->logicalDevice, 1, &inFlightFences[currentFrame]);


    std::vector<VkCommandBuffer> submitCommandBuffers =
    { commandBuffers[imageIndex] };

    if (ImGUI::enabled || imgui.startedFrame) {

        imgui.endFrame();
        imgui.drawn = true;

        rebuildCommandBuffer(imageIndex, camera, gameObjects, lightDescs);

        submitCommandBuffers.emplace_back(imgui.commandBuffers[imageIndex]);
    }

    VkSemaphore iAS = imageAvailableSemaphores[currentFrame];
    VkSemaphore rFS = renderFinishedSemaphores[currentFrame];
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo = Initialisers::submitInfo(
        submitCommandBuffers.data(), static_cast<uint32_t>(submitCommandBuffers.size()), &iAS, 1, &rFS, 1, waitStages);


    if (vkQueueSubmit(deviceContext->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = { swapChain.vkSwapChain };
    VkPresentInfoKHR presentInfo = Initialisers::presentInfoKHR(&rFS, 1, swapChains, 1, &imageIndex);
    auto result = vkQueuePresentKHR(deviceContext->presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || rebuildSwapChain) {
        rebuildSwapChain = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

}

//void RasterRenderer::drawFrame() {

    //VkSemaphore iAS = imageAvailableSemaphores[currentFrame];
    //VkSemaphore rFS = renderFinishedSemaphores[currentFrame];

    //uint32_t imageIndex;
    //VkResult result = vkAcquireNextImageKHR(deviceContext->logicalDevice, swapChain.vkSwapChain, UINT64_MAX, iAS, VK_NULL_HANDLE, &imageIndex);


    //if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    //    recreateSwapChain();
    //    return;
    //}
    //else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    //    throw std::runtime_error("failed to acquire swap chain image!");
    //}

    ////updateUniformBuffer(imageIndex);

    //imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    //vkWaitForFences(deviceContext->logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    //vkResetFences(deviceContext->logicalDevice, 1, &inFlightFences[currentFrame]);


    //std::vector<VkCommandBuffer> submitCommandBuffers =
    //{ commandBuffers[imageIndex] };

    //if (imgui.enabled) {

    //    rebuildCommandBuffer(imageIndex);

    //    submitCommandBuffers.emplace_back(imgui.commandBuffers[imageIndex]);
    //}

    //VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    //VkSubmitInfo submitInfo = Initialisers::submitInfo(
    //    submitCommandBuffers.data(), static_cast<uint32_t>(submitCommandBuffers.size()), &iAS, 1, &rFS, 1, waitStages);


    //if (vkQueueSubmit(deviceContext->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to submit draw command buffer!");
    //}

    //VkSwapchainKHR swapChains[] = { swapChain.vkSwapChain };
    //VkPresentInfoKHR presentInfo = Initialisers::presentInfoKHR(&rFS, 1, swapChains, 1, &imageIndex);
    //result = vkQueuePresentKHR(deviceContext->presentQueue, &presentInfo);


    //vkWaitForFences(deviceContext->logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    //uint32_t imageIndex;
    //VkResult result = vkAcquireNextImageKHR(deviceContext->logicalDevice, swapChain.vkSwapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);


    //if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    //    recreateSwapChain();
    //    return;
    //}
    //else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    //    throw std::runtime_error("failed to acquire swap chain image!");
    //}

    //updateUniformBuffer(imageIndex);

    //imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    //VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
    //VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    //VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };


    //std::array<VkCommandBuffer, 1> submitCommandBuffers =
    //{ commandBuffers[imageIndex] };

    //VkSubmitInfo submitInfo = Initialisers::submitInfo(submitCommandBuffers.data(), static_cast<uint32_t>(submitCommandBuffers.size()), waitSemaphores, 1, signalSemaphores, 1, waitStages);

    //vkResetFences(deviceContext->logicalDevice, 1, &inFlightFences[currentFrame]);

    //if (vkQueueSubmit(deviceContext->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to submit draw command buffer!");
    //}
 
    //VkSwapchainKHR swapChains[] = { swapChain.vkSwapChain };

    //VkPresentInfoKHR presentInfo = Initialisers::presentInfoKHR(signalSemaphores, 1, swapChains, 1, &imageIndex);

    //result = vkQueuePresentKHR(deviceContext->presentQueue, &presentInfo);

    //if (imgui.enabled)
    //{
    //    vkQueueWaitIdle(deviceContext->presentQueue);
    //    buildCommandBuffers();
    //}

    //if (counted == 0)
    //{

    //    Texture dstImage;
    //    dstImage.Create(deviceContext, 800, 600, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //    VkCommandBuffer copyCmd = deviceContext->generateCommandBuffer();

    //    dstImage.insertImageMemoryBarrier(
    //        copyCmd,
    //        0,
    //        VK_ACCESS_TRANSFER_WRITE_BIT,
    //        VK_IMAGE_LAYOUT_UNDEFINED,
    //        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //        VK_PIPELINE_STAGE_TRANSFER_BIT,
    //        VK_PIPELINE_STAGE_TRANSFER_BIT,
    //        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    //    // colorAttachment.image is already in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, and does not need to be transitioned

    //    VkImageCopy imageCopyRegion{};
    //    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    imageCopyRegion.srcSubresource.layerCount = 1;
    //    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    imageCopyRegion.dstSubresource.layerCount = 1;
    //    imageCopyRegion.extent.width = 800;
    //    imageCopyRegion.extent.height = 600;
    //    imageCopyRegion.extent.depth = 1;

    //    vkCmdCopyImage(
    //        copyCmd,
    //        swapChain.images[0].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    //        dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //        1,
    //        &imageCopyRegion);

    //    dstImage.insertImageMemoryBarrier(
    //        copyCmd,
    //        VK_ACCESS_TRANSFER_WRITE_BIT,
    //        VK_ACCESS_MEMORY_READ_BIT,
    //        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //        VK_IMAGE_LAYOUT_GENERAL,
    //        VK_PIPELINE_STAGE_TRANSFER_BIT,
    //        VK_PIPELINE_STAGE_TRANSFER_BIT,
    //        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });


    //    deviceContext->EndCommandBuffer(copyCmd);

    //    // Get layout of the image (including row pitch)
    //    VkImageSubresource subResource{};
    //    subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    VkSubresourceLayout subResourceLayout;

    //    vkGetImageSubresourceLayout(deviceContext->logicalDevice, dstImage.image, &subResource, &subResourceLayout);

    //    // Map image memory so we can start copying from it
    //    vkMapMemory(deviceContext->logicalDevice, dstImage.vkMemory, 0, VK_WHOLE_SIZE, 0, (void**)&imagedata);
    //    imagedata += subResourceLayout.offset;


    //    const char* filename = "headless.ppm";
    //    std::ofstream file(filename, std::ios::out | std::ios::binary);

    //    // ppm header
    //    file << "P6\n" << 800 << "\n" << 600 << "\n" << 255 << "\n";

    //    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    //    // Check if source is BGR and needs swizzle
    //    std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
    //    const bool colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), VK_FORMAT_R8G8B8A8_UNORM) != formatsBGR.end());

    //    // ppm binary pixel data
    //    for (int32_t y = 0; y < 600; y++) {
    //        unsigned int* row = (unsigned int*)imagedata;
    //        for (int32_t x = 0; x < 800; x++) {
    //            if (colorSwizzle) {
    //                file.write((char*)row + 2, 1);
    //                file.write((char*)row + 1, 1);
    //                file.write((char*)row, 1);
    //            }
    //            else {
    //                file.write((char*)row, 3);
    //            }
    //            row++;
    //        }
    //        imagedata += subResourceLayout.rowPitch;
    //    }
    //    file.close();

    //    printImage = false;

    //    vkUnmapMemory(deviceContext->logicalDevice, dstImage.vkMemory);
    //    dstImage.Destroy();
    //    printImage = true;
    //    counted--;
    //}

    //if(!printImage)
    //    counted--;

    //if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.framebufferResized) {
    //    window.framebufferResized = false;
    //    recreateSwapChain();
    //}
    //else if (result != VK_SUCCESS) {
    //    throw std::runtime_error("failed to present swap chain image!");
    //}

    //currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
//}