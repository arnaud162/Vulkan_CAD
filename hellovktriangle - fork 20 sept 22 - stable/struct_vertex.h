#ifndef STRUCT_VERTEX_H
#define STRUCT_VERTEX_H

#include <QVector2D>
#include <QVulkanWindow>
struct Vertex {
    QVector3D pos;
   QVector3D color;
    QVector2D texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0; // position
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

//        attributeDescriptions[1].binding = 1; // normal
//        attributeDescriptions[1].location = 0;
//        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[1].offset = 5 * sizeof(float);

//        attributeDescriptions[2].binding = 2; // instTranslate
//        attributeDescriptions[2].location = 1;
//        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[2].offset = 0;

//        attributeDescriptions[3].binding = 3; // instDiffuseAdjust
//        attributeDescriptions[3].location = 1;
//        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[3].offset = 3 * sizeof(float);



        attributeDescriptions[1].binding = 0;            // ce systéme de couleurs était utilisé quand il n'y avait pas de diffusion de la lumiére
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;                         //textures pas d'actualité pour le moment
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

//    QVector3D returnColor(){
//        return color;
//    }
};
#endif // STRUCT_VERTEX_H
