#include "vertex.h"

#include <SDL.h>

VkVertexInputBindingDescription vertex_get_binding_description(){
    VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
    return bindingDescription;
}

void vertex_get_attribute_descriptions(VkVertexInputAttributeDescription *descriptions, size_t count){
    if(!descriptions || count < 2) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "get_vertex_attribute_descriptions: invalid array or count < 2");
        return; // Early exit to prevent buffer overflow
    }

    // Attribute 0: Position
    descriptions[0].binding = 0;
    descriptions[0].location = 0;
    descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    descriptions[0].offset = offsetof(Vertex, pos);

    // Attribute 1: Color
    descriptions[1].binding = 0;
    descriptions[1].location = 1;
    descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[1].offset = offsetof(Vertex, color);
}