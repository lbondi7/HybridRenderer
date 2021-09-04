#pragma once
#include "Constants.h"

struct MemoryData {
    VkDeviceMemory memory;
    VkDeviceSize currentOffset;
    VkDeviceSize size = 0;
    uint32_t memoryType = 0;
    VkMemoryPropertyFlags properties;
    VkMemoryAllocateFlags flags;
    uint32_t id = 0;
};

struct BufferData {
    MemoryData* memoryData;
    VkBuffer buffer;
    VkDeviceSize offset;
    VkDeviceSize currentMemoryOffset = 0;
    VkDeviceSize memStartOffset = 0;
    VkDeviceSize size = 0;
    VkDeviceSize allocatedSize = 0;
    VkBufferUsageFlags usage;
    uint32_t id = 0;
    void* mapped = nullptr;
};


struct BufferInfo {
    MemoryData* memoryData;
    VkBuffer buffer;
    VkDeviceSize offset = 0;
    VkDeviceSize memOffset = 0;
    VkDeviceSize size = 0;
    VkBufferUsageFlags usage;
};


class Allocator
{
public:
    Allocator() = default;
    ~Allocator();


    void init(VkDevice _logicalDevice, VkPhysicalDevice physicalDevice);

    void allocateBuffer(BufferInfo& bufferInfo, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    void getBuffer(BufferInfo& bufferInfo, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    MemoryData* getMemory(const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage);

    void destroy();

    std::vector<MemoryData> memoryPool;
    std::vector<BufferData> bufferPool;


    VkDevice logicalDevice;
    VkPhysicalDevice physicalDevice;

    VkPhysicalDeviceProperties properties;

    VkDeviceSize baseMemorySize = 256000000;
    VkDeviceSize baseBufferSize = 64000000;

    uint32_t availableMemoryID = 0;
    uint32_t availableBufferID = 0;

};

