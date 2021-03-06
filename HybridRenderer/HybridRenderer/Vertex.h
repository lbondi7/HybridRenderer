#pragma once

#include "Constants.h"
#include "Initilizers.h"

#include <array>

struct Vertex {

    glm::vec3 pos = glm::vec3(0.0);;
    glm::vec2 texCoord = glm::vec2(0.5);
    //glm::vec3 color = glm::vec3(1.0);
    glm::vec3 normal = glm::vec3(0.0, 1.0, 0.0);

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = Initialisers::vertexInputBindingDescription(0, sizeof(Vertex));

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{
        Initialisers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)),
        Initialisers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)),
        Initialisers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
        };                                                 

        return attributeDescriptions;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(const std::vector<VertexAttributes>& attributes) {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.reserve(3);
        for (auto attribute : attributes)
        {
            if (attribute == VertexAttributes::POSITION)
            {
                attributeDescriptions.emplace_back(Initialisers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)));
            }
            if (attribute == VertexAttributes::UV_COORD)
            {
                attributeDescriptions.emplace_back(Initialisers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)));
            }
            //if (attribute == VertexAttributes::V_COLOUR)
            //{
            //    attributeDescriptions.emplace_back(Initialisers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)));
            //}
            if (attribute == VertexAttributes::NORMAL)
            {
                attributeDescriptions.emplace_back(Initialisers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)));
            }
        }

        attributeDescriptions.shrink_to_fit();
        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && texCoord == other.texCoord && normal == other.normal;
    }
};


namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return (hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec2>()(vertex.texCoord) << 1) >> 1) ^ 
                (hash<glm::vec3>()(vertex.normal) << 1);
        }
    };
}
