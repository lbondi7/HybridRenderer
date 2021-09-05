#include "Buffer.h"

#include "Utility.h"
#include "Initilizers.h"

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

    createBuffer(size, usage, properties);

    if (_data)
        Map(_data);
}

void Buffer::Create(DeviceContext* _devices, VkDeviceSize _size, void* _data)
{
    deviceContext = _devices;
    size = _size;
    usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

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

    Allocate(size, usage, properties);

    if (_data)
        AllocatedMap(_data);
}

void Buffer::Allocate(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {

    deviceContext->allocator.allocateBuffer(bufferInfo, size, usage, properties);

    descriptorInfo = Initialisers::descriptorBufferInfo(bufferInfo.buffer, bufferInfo.size, bufferInfo.offset);
}


void Buffer::CopyFrom(Buffer* other) {

    VkCommandBuffer commandBuffer = deviceContext->generateCommandBuffer();

    VkBufferCopy copyRegion = Initialisers::bufferCopy(size);
    vkCmdCopyBuffer(commandBuffer, other->vkBuffer, vkBuffer, 1, &copyRegion);

    deviceContext->EndCommandBuffer(commandBuffer);
}

void Buffer::AllocatedCopyFrom(Buffer* other) {

    VkCommandBuffer commandBuffer = deviceContext->generateCommandBuffer();

    VkBufferCopy copyRegion = Initialisers::bufferCopy(other->size, other->bufferInfo.offset, bufferInfo.offset);
    vkCmdCopyBuffer(commandBuffer, other->vkBuffer, bufferInfo.buffer, 1, &copyRegion);

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

    vkMapMemory(deviceContext->logicalDevice, bufferInfo.memoryData->memory, bufferInfo.memOffset, descriptorInfo.range, 0, &data);
    memcpy(data, src_data, static_cast<size_t>(descriptorInfo.range));
    vkUnmapMemory(deviceContext->logicalDevice, bufferInfo.memoryData->memory);
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
    VkBufferDeviceAddressInfoKHR buffer_device_address_info{};
    buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    buffer_device_address_info.buffer = vkBuffer;
    return vkGetBufferDeviceAddressKHR(deviceContext->logicalDevice, &buffer_device_address_info);
}

//
//uint64_t Buffer::GetDeviceAddress2()
//{
//    VkBufferDeviceAddressInfoKHR buffer_device_address_info{};
//    buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
//    buffer_device_address_info.buffer = bufferInfo.buffer;
//    return vkGetBufferDeviceAddressKHR(deviceContext->logicalDevice, &buffer_device_address_info);
//}