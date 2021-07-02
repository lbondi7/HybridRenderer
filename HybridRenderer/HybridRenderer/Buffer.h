#pragma once

#include "Constants.h"
#include "Device.h"

class Buffer
{
public:
	Buffer() = default;
	~Buffer();

	void Create(DeviceContext* _devices, VkDeviceSize _size, VkBufferUsageFlags _usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VkMemoryPropertyFlags _properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, void* _data = nullptr);

	void Create(DeviceContext* _devices, VkDeviceSize _size, void* _data);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

	void Create2(DeviceContext* _devices, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, void* _data = nullptr);

	void createBuffer2(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

	void CopyFrom(Buffer* other);

	void CopyFrom2(Buffer* other);

	void Destroy();

	void Map(void* _data);

	void Map();

	void Unmap();

	void Map2(const void* src_data);

	void Flush(VkDeviceSize size = VK_NULL_HANDLE, VkDeviceSize offset = 0);

	VkBuffer vkBuffer;
	VkDeviceMemory memory;
	VkDeviceSize size;
	VkBufferUsageFlags usage; 
	VkMemoryPropertyFlags properties;
	void* data = nullptr;
	bool mapped = false;

	VkDescriptorBufferInfo descriptorInfo;

	BufferInfo bufferInfo;

private:

	DeviceContext* devices;

};


