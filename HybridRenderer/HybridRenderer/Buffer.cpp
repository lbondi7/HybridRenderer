#include "Buffer.h"

#include "Utility.h"
#include "Initilizers.h"

Buffer::~Buffer()
{
	devices = nullptr;
}

void Buffer::Create(DeviceContext* _devices, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, void* _data)
{
    devices = _devices;
    size = _size;
    usage = _usage;
    properties = _properties;


    createBuffer(size, usage, properties);

    if (_data)
        Map(_data);
}

void Buffer::Create(DeviceContext* _devices, VkDeviceSize _size, void* _data)
{
    devices = _devices;
    size = _size;
    usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    createBuffer(size, usage, properties);
    Map(_data);
}

void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {

    VkBufferCreateInfo bufferInfo = Initialisers::bufferCreateInfo(size, usage);
    //bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    //bufferInfo.size = size;
    //bufferInfo.usage = usage;
    //bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(devices->logicalDevice, &bufferInfo, nullptr, &vkBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(devices->logicalDevice, vkBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = Initialisers::memoryAllocateInfo(memRequirements.size,
        Utility::findMemoryType(memRequirements.memoryTypeBits, devices->physicalDevice, properties));
    //allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    //allocInfo.allocationSize = memRequirements.size;
    //allocInfo.memoryTypeIndex = Utility::findMemoryType(memRequirements.memoryTypeBits, devices->physicalDevice, properties);

    if (vkAllocateMemory(devices->logicalDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }


    vkBindBufferMemory(devices->logicalDevice, vkBuffer, memory, 0);

    descriptorInfo = Initialisers::descriptorBufferInfo(vkBuffer, size);
}

void Buffer::Create2(DeviceContext* _devices, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, void* _data)
{
    devices = _devices;
    size = _size;
    usage = _usage;
    properties = _properties;


    createBuffer2(size, usage, properties);

    if (_data)
        Map2(_data);
}

void Buffer::createBuffer2(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {

    devices->allocator.allocateBuffer(bufferInfo, size, usage, properties);

    descriptorInfo = Initialisers::descriptorBufferInfo(bufferInfo.buffer, bufferInfo.size, bufferInfo.offset);
}


void Buffer::CopyFrom(Buffer* other) {

    VkCommandBuffer commandBuffer = devices->generateCommandBuffer();

    VkBufferCopy copyRegion = Initialisers::bufferCopy(size);
    //copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, other->vkBuffer, vkBuffer, 1, &copyRegion);

    devices->EndCommandBuffer(commandBuffer);
}

void Buffer::CopyFrom2(Buffer* other) {

    VkCommandBuffer commandBuffer = devices->generateCommandBuffer();

    VkBufferCopy copyRegion{};
    copyRegion.size = other->size;
    copyRegion.srcOffset = other->bufferInfo.offset;
    copyRegion.dstOffset = bufferInfo.offset;
    vkCmdCopyBuffer(commandBuffer, other->vkBuffer, bufferInfo.buffer, 1, &copyRegion);


    devices->EndCommandBuffer(commandBuffer);
}


void Buffer::Destroy() {

    Unmap();
    if (vkBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(devices->logicalDevice, vkBuffer, nullptr);
    }
    if (memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(devices->logicalDevice, memory, nullptr);
    }
}

void Buffer::Map(void* _data) {

    vkMapMemory(devices->logicalDevice, memory, 0, size, 0, &data);
    memcpy(data, _data, (size_t)size);
    vkUnmapMemory(devices->logicalDevice, memory);
}

void Buffer::Map()
{
    vkMapMemory(devices->logicalDevice, memory, 0, size, 0, &data);
    mapped = true;
}

void Buffer::Unmap()
{
    if (mapped)
        vkUnmapMemory(devices->logicalDevice, memory);

    mapped = false;
}

void Buffer::Map2(const void* src_data) {

    vkMapMemory(devices->logicalDevice, bufferInfo.memoryData->memory, bufferInfo.memOffset, descriptorInfo.range, 0, &data);
    memcpy(data, src_data, static_cast<size_t>(descriptorInfo.range));
    vkUnmapMemory(devices->logicalDevice, bufferInfo.memoryData->memory);
}

void Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    vkFlushMappedMemoryRanges(devices->logicalDevice, 1, &mappedRange);
}
