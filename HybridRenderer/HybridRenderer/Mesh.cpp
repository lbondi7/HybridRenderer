#include "Mesh.h"

Mesh::~Mesh()
{
}

void Mesh::Init(DeviceContext* _devices)
{
    devices = _devices;
	vertexBuffer = std::make_unique<Buffer>();
	indexBuffer = std::make_unique<Buffer>();

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    Buffer stagingBuffer;
    stagingBuffer.Create(devices, bufferSize, vertices.data());

    vertexBuffer->Create2(devices, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vertexBuffer->CopyFrom2(&stagingBuffer);

    bufferSize = sizeof(indices[0]) * indices.size();

    Buffer stagingIndexBuffer;
    stagingIndexBuffer.Create(devices, bufferSize, indices.data());

    indexBuffer->Create2(devices, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    indexBuffer->CopyFrom2(&stagingIndexBuffer);

    stagingBuffer.Destroy();
    stagingIndexBuffer.Destroy();


    DescriptorSetRequest request{};
    request.ids.emplace_back(std::pair<uint32_t, VkDescriptorType>(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
    for (size_t i = 0; i < devices->imageCount; i++) {

        request.data.push_back(&texture->descriptorInfo);
    }
    devices->dsm.getDescriptor(descriptor, request);
}

void Mesh::Destroy() {
    vertexBuffer->Destroy();
    indexBuffer->Destroy();
    texture = nullptr;
    material = nullptr;
}

void Mesh::Bind(VkCommandBuffer cmdBuffer)
{

    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer->bufferInfo.buffer, &vertexBuffer->bufferInfo.offset);

    vkCmdBindIndexBuffer(cmdBuffer, indexBuffer->bufferInfo.buffer, indexBuffer->bufferInfo.offset, VK_INDEX_TYPE_UINT32);
}
