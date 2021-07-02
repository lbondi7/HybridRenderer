#include "Allocator.h"

#include "Utility.h"

Allocator::~Allocator()
{
}

void Allocator::init(VkDevice _logicalDevice, VkPhysicalDevice _physicalDevice)
{
	logicalDevice = _logicalDevice;
	physicalDevice = _physicalDevice;

	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
}

void Allocator::allocateBuffer(BufferInfo& bufferInfo, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	getBuffer(bufferInfo, size, usage, properties);
}

void Allocator::getBuffer(BufferInfo& bufferInfo, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties) {


    auto minAlignment = properties.limits.minUniformBufferOffsetAlignment;
    int amount = static_cast<int>(std::ceil(static_cast<float>(size) / static_cast<float>(minAlignment)));
    minAlignment = minAlignment * static_cast<VkDeviceSize>(amount);

    for (auto& buffer : bufferPool) {
        if (usage == buffer.usage && (minAlignment + buffer.size <= maxBufferSize)) {

            bufferInfo.buffer = buffer.buffer;
            bufferInfo.memoryData = buffer.memoryData;
            bufferInfo.offset = buffer.size;
            bufferInfo.memOffset = buffer.memoryData->currentOffset;
            bufferInfo.usage = usage;
            bufferInfo.size = size;

            buffer.size += minAlignment;
            buffer.memoryData->currentOffset += minAlignment;

            return;
        }
    }

    auto& buffer = bufferPool.emplace_back();
    buffer.usage = usage;
    buffer.id = availableBufferID++;

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = maxBufferSize > size ? maxBufferSize : size;
    bufferCreateInfo.usage = usage;

    if (vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &buffer.buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(logicalDevice, buffer.buffer, &memRequirements);

    auto memoryData = getMemory(memRequirements, memProperties);

    vkBindBufferMemory(logicalDevice, buffer.buffer, memoryData->memory, 0);
    buffer.size += minAlignment;
    buffer.memoryData = memoryData;

    bufferInfo.buffer = buffer.buffer;
    bufferInfo.memoryData = buffer.memoryData;
    bufferInfo.offset = 0;
    bufferInfo.memOffset = buffer.memoryData->currentOffset;
    bufferInfo.usage = usage;
    bufferInfo.size = size;

    memoryData->currentOffset += minAlignment;
}


MemoryData* Allocator::getMemory(const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties) {

    for (auto& mem : memoryPool) {
        if (mem.properties == properties &&
            memRequirements.size + mem.currentOffset < maxMemorySize) {
            return &mem;
        }
    }
    auto& mem = memoryPool.emplace_back();
    mem.id = availableMemoryID++;
    mem.properties = properties;
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = maxMemorySize > memRequirements.size ? maxMemorySize : memRequirements.size;
    allocInfo.memoryTypeIndex = Utility::findMemoryType(memRequirements.memoryTypeBits, physicalDevice, properties);

    if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &mem.memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    return &mem;
}

void Allocator::destroy() {

    for (auto& buffer : bufferPool) {
        buffer.memoryData = nullptr;
        if (buffer.buffer)
        {
            vkDestroyBuffer(logicalDevice, buffer.buffer, nullptr);
            buffer.buffer = nullptr;
        }
    }

    for (auto& memory : memoryPool) {
        if (memory.memory)
        {
            vkFreeMemory(logicalDevice, memory.memory, nullptr);
            memory.memory = nullptr;
        }
    }

    logicalDevice = VK_NULL_HANDLE;
}

