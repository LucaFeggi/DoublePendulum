#ifndef RENDERER_VK_VERTEX_H
#define RENDERER_VK_VERTEX_H

#include <vulkan/vulkan.h>

typedef struct{
    float x, y;
}Vec2f; // 2D float vector (position)

typedef struct{ 
    float r, g, b;
}Vec3f; // 3D float vector (color)

typedef struct Vertex{
    Vec2f pos;   // location = 0
    Vec3f color; // location = 1
}Vertex;

VkVertexInputBindingDescription vertex_get_binding_description();
void vertex_get_attribute_descriptions(VkVertexInputAttributeDescription *descriptions, size_t count);

#endif // RENDERER_VK_VERTEX_H
