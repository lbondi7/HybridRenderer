#include "Buffer.h"

#include "Utility.h"
#include "Initilizers.h"
#include "DebugLogger.h"

Buffer::~Buffer()
{
	deviceContext = nullptr;
}

void Buffer::Create(DeviceContext* _devices, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, void* _data)
{
    deviceContext = _devices;
    size = _size;
    usage = _usage;
    properties = _properties;

    vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkGetBufferDeviceAddressKHR"));

    if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    {
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    createBuffer(size, usage, properties);

    if (_data)
    {
        if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            Stage(_data);
        }
        else if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            Map(_data);
        }
    }
}

void Buffer::Create(DeviceContext* _devices, VkDeviceSize _size, void* _data)
{
    deviceContext = _devices;
    size = _size;
    usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    offset = 0;
    vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkGetBufferDeviceAddressKHR"));

    createBuffer(size, usage, properties);
    Map(_data);
}

void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {

    VkBufferCreateInfo bufferInfo = Initialisers::bufferCreateInfo(size, usage);

    if (vkCreateBuffer(deviceContext->logicalDevice, &bufferInfo, nullptr, &vkBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(deviceContext->logicalDevice, vkBuffer, &memRequirements);


    VkMemoryAllocateInfo allocInfo = Initialisers::memoryAllocateInfo(memRequirements.size,
        Utility::findMemoryType(memRequirements.memoryTypeBits, deviceContext->physicalDevice, properties));

    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        allocInfo.pNext = &allocFlagsInfo;
    }

    if (vkAllocateMemory(deviceContext->logicalDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(deviceContext->logicalDevice, vkBuffer, memory, 0);

    descriptorInfo = Initialisers::descriptorBufferInfo(vkBuffer, size);
}

void Buffer::Allocate(DeviceContext* _devices, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, void* _data)
{
    deviceContext = _devices;
    size = _size;
    usage = _usage;
    properties = _properties;

    if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    {
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    Allocate(size, usage, properties);

    if (_data)
    {
        if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            AllocatedStage(_data);
        }
        else if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            AllocatedMap(_data);
        }
    }
}

void Buffer::Allocate(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {

    deviceContext->allocator.allocateBuffer(bufferInfo, size, usage, properties);

    descriptorInfo = Initialisers::descriptorBufferInfo(bufferInfo.buffer, bufferInfo.size, bufferInfo.offset);
}

void Buffer::Stage(void* data) {

    Buffer stagingBuffer;
    stagingBuffer.Create(deviceContext, size, data);

    CopyFrom(&stagingBuffer);
    stagingBuffer.Destroy();
}

void Buffer::AllocatedStage(void* data)
{
    Buffer stagingBuffer;
    stagingBuffer.Create(deviceContext, size, data);

    AllocatedCopyFrom(&stagingBuffer);
    stagingBuffer.Destroy();

}

void Buffer::CopyFrom(Buffer* other) {

    VkCommandBuffer commandBuffer = deviceContext->generateCommandBuffer();

    VkBufferCopy copyRegion = Initialisers::bufferCopy(size);
    vkCmdCopyBuffer(commandBuffer, other->vkBuffer, vkBuffer, 1, &copyRegion);

    deviceContext->EndCommandBuffer(commandBuffer);
}

void Buffer::AllocatedCopyFrom(Buffer* other) {

    VkCommandBuffer commandBuffer = deviceContext->generateCommandBuffer();

    auto& bufferData = deviceContext->allocator.getBuffer(bufferInfo.id);

    VkBufferCopy copyRegion = Initialisers::bufferCopy(other->size, other->offset, bufferInfo.offset);
    vkCmdCopyBuffer(commandBuffer, other->vkBuffer, bufferData.buffer, 1, &copyRegion);

    deviceContext->EndCommandBuffer(commandBuffer);
}

void Buffer::Destroy() {

    Unmap();
    if (vkBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(deviceContext->logicalDevice, vkBuffer, nullptr);
    }
    if (memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(deviceContext->logicalDevice, memory, nullptr);
        //deviceContext->allocator.memoryAllocCount--;
        //Log(deviceContext->allocator.memoryAllocCount, "Memory Allocation Count");

    }
}

void Buffer::Map(void* _data) {

    vkMapMemory(deviceContext->logicalDevice, memory, 0, size, 0, &data);
    memcpy(data, _data, (size_t)size);
    vkUnmapMemory(deviceContext->logicalDevice, memory);
}

void Buffer::Map()
{
    vkMapMemory(deviceContext->logicalDevice, memory, 0, size, 0, &data);
    mapped = true;
}

void Buffer::Unmap()
{
    if (mapped)
        vkUnmapMemory(deviceContext->logicalDevice, memory);

    mapped = false;
}

void Buffer::AllocatedMap(const void* src_data) {

    auto& memoryData = deviceContext->allocator.getMemory(bufferInfo.memoryID);
    vkMapMemory(deviceContext->logicalDevice, memoryData.memory, bufferInfo.memOffset, descriptorInfo.range, 0, &data);
    memcpy(data, src_data, static_cast<size_t>(descriptorInfo.range));
    vkUnmapMemory(deviceContext->logicalDevice, memoryData.memory);
}

void Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    vkFlushMappedMemoryRanges(deviceContext->logicalDevice, 1, &mappedRange);
}

uint64_t Buffer::GetDeviceAddress()
{
    VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
    bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAddressInfo.buffer = vkBuffer;
   
    return vkGetBufferDeviceAddressKHR(deviceContext->logicalDevice, &bufferDeviceAddressInfo);
}

//
uint64_t Buffer::GetAllocatedDeviceAddress()
{
    auto& bufferData = deviceContext->allocator.getBuffer(bufferInfo.id);
    VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
    bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAddressInfo.buffer = bufferData.buffer;
    return vkGetBufferDeviceAddressKHR(deviceContext->logicalDevice, &bufferDeviceAddressInfo);
}