#include "RenderPass.h"

#include "Utility.h"

#include "Initilizers.h"

#include <thread>
#include <array>
#include <stdexcept>

RenderPass::~RenderPass()
{
    devices = nullptr;
    swapChain = nullptr;
}

//void RenderPass::Create(Device* _devices, SwapChain* _swapChain, OffscreenPass& offscreenPass)
//{
//    devices = _devices;
//    swapChain = _swapChain;
//
//    Init(offscreenPass);
//}

void RenderPass::Begin(VkCommandBuffer cmdBuffer, VkFramebuffer frameBuffer, VkExtent2D extent, const VkClearValue* pClearValues, uint32_t clearValueCount)
{

    VkRenderPassBeginInfo renderPassBeginInfo = Initialisers::renderPassBeginInfo(vkRenderPass, frameBuffer,
        extent, clearValueCount, pClearValues);

    vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::End(VkCommandBuffer cmdBuffer)
{

    vkCmdEndRenderPass(cmdBuffer);
}

void RenderPass::Create(DeviceContext* _devices, SwapChain* _swapChain)
{
    devices = _devices;
    swapChain = _swapChain;
}


void RenderPass::Destroy() {
    vkDestroyRenderPass(devices->logicalDevice, vkRenderPass, nullptr);
}
