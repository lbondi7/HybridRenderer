#include "Mesh.h"

Mesh::~Mesh()
{
}

void Mesh::Init(Device* _devices)
{
    devices = _devices;
	vertexBuffer = std::make_unique<Buffer>();
	indexBuffer = std::make_unique<Buffer>();

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    Buffer stagingBuffer;
    stagingBuffer.Create(devices, bufferSize, vertices.data());

    vertexBuffer->Create(devices, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vertexBuffer->CopyFrom(&stagingBuffer);

    bufferSize = sizeof(indices[0]) * indices.size();

    Buffer stagingIndexBuffer;
    stagingIndexBuffer.Create(devices, bufferSize, indices.data());

    indexBuffer->Create(devices, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    indexBuffer->CopyFrom(&stagingIndexBuffer);

    stagingBuffer.Destroy();
    stagingIndexBuffer.Destroy();

}

void Mesh::Destroy() {
    vertexBuffer->Destroy();
    indexBuffer->Destroy();
}
