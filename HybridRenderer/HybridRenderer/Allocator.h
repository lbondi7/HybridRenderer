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
    VkBuffer buffer;
    VkDeviceSize offset;
    VkDeviceSize currentMemoryOffset = 0;
    VkDeviceSize memStartOffset = 0;
    VkDeviceSize size = 0;
    VkDeviceSize allocatedSize = 0;
    VkBufferUsageFlags usage;
    uint32_t id = 0;
    uint32_t memoryID = 0;
    void* mapped = nullptr;
};


struct BufferInfo {
    VkBuffer buffer;
    VkDeviceSize offset = 0;
    VkDeviceSize memOffset = 0;
    VkDeviceSize size = 0;
    VkBufferUsageFlags usage;
    uint32_t id = 0;
    uint32_t memoryID = 0;
};


class Allocator
{
public:
    Allocator() = default;
    ~Allocator();


    void init(VkDevice _logicalDevice, VkPhysicalDevice physicalDevice, 
        const VkPhysicalDeviceProperties& physicalDeviceProperties,
    const VkPhysicalDeviceProperties2& physicalDevicePropertiesExt, 
        size_t defaultMemoryAllocations = 100, size_t defaultBufferAllocations = 100);

    void allocateBuffer(BufferInfo& bufferInfo, VkDeviceSize size, 
        VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    void getBuffer(BufferInfo& bufferInfo, VkDeviceSize size, 
        VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    MemoryData* getMemory(const VkMemoryRequirements& memRequirements, 
        VkMemoryPropertyFlags properties, VkBufferUsageFlags usage);

    MemoryData& getMemory(uint32_t memoryID);

    BufferData& getBuffer(uint32_t bufferID);

    void destroy();


    uint32_t memoryAllocCount = 0;

private:
    std::vector<MemoryData> memoryPool;
    std::vector<BufferData> bufferPool;


    VkDevice logicalDevice;
    VkPhysicalDevice physicalDevice;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceProperties2 propertiesExt;
    //VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties;

    VkDeviceSize baseMemorySize = 256000000;
    VkDeviceSize baseBufferSize = 256000;

    uint32_t availableMemoryID = 0;
    uint32_t availableBufferID = 0;

};

