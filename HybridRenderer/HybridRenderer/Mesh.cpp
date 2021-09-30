#include "Mesh.h"

Mesh::~Mesh()
{
}

void Mesh::Init(DeviceContext* _devices)
{
    devices = _devices;

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    //Buffer stagingBuffer;
    //stagingBuffer.Create(devices, bufferSize, vertices.data());

    //vertexBuffer.Create(devices, bufferSize, 
    //    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
    //    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    //    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertices.data());
    vertexBuffer.Create(devices, bufferSize, 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertices.data());

    bufferSize = sizeof(indices[0]) * indices.size();

    //indexBuffer.Create(devices, bufferSize, 
    //    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
    //    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    //    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indices.data());
    indexBuffer.Create(devices, bufferSize, 
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indices.data());
    //indexBuffer.AllocatedCopyFrom(&stagingIndexBuffer);

    //stagingBuffer.Destroy();
    //stagingIndexBuffer.Destroy();


    min = glm::vec3(FLT_MAX);
    max = glm::vec3(-FLT_MAX);

    for (auto& vertex : vertices)
    {
        min.x = std::min(min.x, vertex.pos.x);
        min.y = std::min(min.y, vertex.pos.y);
        min.z = std::min(min.z, vertex.pos.z);
        max.x = std::max(max.x, vertex.pos.x);
        max.y = std::max(max.y, vertex.pos.y);
        max.z = std::max(max.z, vertex.pos.z);
    }
}

void Mesh::Destroy() {
    vertexBuffer.Destroy();
    indexBuffer.Destroy();
    texture = nullptr;
    //material = nullptr;
}

void Mesh::Bind(VkCommandBuffer cmdBuffer)
{
    if (vertexBuffer.vkBuffer) 
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer.vkBuffer, &vertexBuffer.offset);
    else
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer.bufferInfo.buffer, &vertexBuffer.bufferInfo.offset);

    if(indexBuffer.vkBuffer)
        vkCmdBindIndexBuffer(cmdBuffer, indexBuffer.vkBuffer, indexBuffer.offset, VK_INDEX_TYPE_UINT32);
    else 
        vkCmdBindIndexBuffer(cmdBuffer, indexBuffer.bufferInfo.buffer, indexBuffer.bufferInfo.offset, VK_INDEX_TYPE_UINT32);
}
