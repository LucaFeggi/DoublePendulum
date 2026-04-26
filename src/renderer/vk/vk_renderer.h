#ifndef RENDERER_VK_VK_RENDERER_H
#define RENDERER_VK_VK_RENDERER_H

#include "../../app/window.h"

#include <vulkan/vulkan.h>

typedef struct{
    VkPipelineLayout layout;
    VkPipeline pipeline;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
}PipelineContext;

typedef struct{
    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkImage *swapChainImages;
    uint32_t swapChainImagesCount;
    VkImageView *swapChainImageViews;

    VkFramebuffer *swapChainFramebuffers;

    VkRenderPass renderPass;

    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;

    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;

    uint32_t maxFramesInFlight;
    uint32_t currentFrame;

    PipelineContext pipeline_pendulum;

}VkRenderer;

bool vk_renderer_init(VkRenderer *renderer, const Window *window);
void vk_renderer_free(VkRenderer *renderer);
void vk_renderer_render(VkRenderer *renderer, const Window *window);

#endif // RENDERER_VK_VK_RENDERER_H
