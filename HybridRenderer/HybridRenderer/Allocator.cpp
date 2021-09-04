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
        if (usage == buffer.usage && (minAlignment + buffer.size <= baseBufferSize)) {

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
    bufferCreateInfo.size = baseBufferSize > size ? baseBufferSize : size;
    bufferCreateInfo.usage = usage;

    if (vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &buffer.buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(logicalDevice, buffer.buffer, &memRequirements);

    auto memoryData = getMemory(memRequirements, memProperties, usage);
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


MemoryData* Allocator::getMemory(const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage) {

    for (auto& memory : memoryPool) {
        if (usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        {
            if (memory.flags == VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR && memory.properties & properties &&
                memRequirements.size + memory.size < baseMemorySize) {
                return &memory;
            }
            continue;
        }

        if (memory.properties & properties &&
            memRequirements.size + memory.size < baseMemorySize) {
            return &memory;
        }
    }
    auto& memory = memoryPool.emplace_back();
    memory.id = availableMemoryID++;
    memory.properties = properties;
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = baseMemorySize > memRequirements.size ? baseMemorySize : memRequirements.size;
    allocInfo.memoryTypeIndex = Utility::findMemoryType(memRequirements.memoryTypeBits, physicalDevice, properties);
    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
        memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        memoryAllocateFlagsInfo.flags = memory.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        allocInfo.pNext = &memoryAllocateFlagsInfo;
    }

    if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &memory.memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    return &memory;
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

