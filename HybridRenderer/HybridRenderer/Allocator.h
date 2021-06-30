#pragma once
#include "Constants.h"

struct MemoryData {
    VkDeviceMemory memory;
    VkDeviceSize currentOffset;
    VkDeviceSize maxSize = 268435456;
    VkDeviceSize size = 0;
    uint32_t memoryType = 0;
    VkMemoryPropertyFlags properties;
    uint32_t id = 0;
};

struct BufferData {
    MemoryData* memoryData;
    VkBuffer vk_buffer;
    VkDeviceSize offset;
    VkDeviceSize maxSize = 67108864;
    VkDeviceSize size = 0;
    VkBufferUsageFlags usage;
    uint32_t id;
    void* mapped = nullptr;
};


class Allocator
{
public:
    Allocator() = default;
    ~Allocator();



    std::vector<MemoryData> memoryPool;
    std::vector<BufferData> bufferPool;
};

