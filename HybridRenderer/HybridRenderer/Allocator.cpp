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

    VkDeviceSize minAlignment = VK_WHOLE_SIZE;
    if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    {
        minAlignment = properties.limits.minUniformBufferOffsetAlignment;
        int amount = static_cast<int>(std::ceil(static_cast<float>(size) / static_cast<float>(minAlignment)));
        minAlignment = minAlignment * static_cast<VkDeviceSize>(amount);
    }
    else {
        minAlignment = size;
    }
    for (auto& buffer : bufferPool) {
        if (usage == buffer.usage && (minAlignment + buffer.size <= maxBufferSize)) {

            bufferInfo.buffer = buffer.buffer;
            bufferInfo.memoryData = buffer.memoryData;
            bufferInfo.offset = buffer.size;
            bufferInfo.memOffset += buffer.memStartOffset + buffer.currentMemoryOffset;
            bufferInfo.usage = usage;
            bufferInfo.size = size;

            buffer.size += minAlignment;
            buffer.currentMemoryOffset += minAlignment;

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
    vkBindBufferMemory(logicalDevice, buffer.buffer, memoryData->memory, memoryData->size);
    buffer.size += minAlignment;
    buffer.memoryData = memoryData;
    buffer.memStartOffset = memoryData->size;
    buffer.currentMemoryOffset += minAlignment;

    bufferInfo.buffer = buffer.buffer;
    bufferInfo.memoryData = buffer.memoryData;
    bufferInfo.offset = 0;
    bufferInfo.memOffset = 0;
    bufferInfo.usage = usage;
    bufferInfo.size = size;

    memoryData->size += memRequirements.size;
}


MemoryData* Allocator::getMemory(const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties) {

    for (auto& mem : memoryPool) {
        if (mem.properties& properties&&
            memRequirements.size + mem.size < maxMemorySize) {
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
            buffer.buffer = VK_NULL_HANDLE;
        }
    }

    for (auto& memory : memoryPool) {
        if (memory.memory)
        {
            vkFreeMemory(logicalDevice, memory.memory, nullptr);
            memory.memory = VK_NULL_HANDLE;
        }
    }

    logicalDevice = VK_NULL_HANDLE;
}

