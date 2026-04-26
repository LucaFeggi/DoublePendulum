#ifndef VK_UTILS_H
#define VK_UTILS_H

#include "vk_renderer.h"

#include "../app/window.h"

#include <stdbool.h>

#include <vulkan/vulkan.h>

#define QUEUE_FAMILY_INDEX_INVALID -1

static const char *DEVICE_EXTENSION_LIST[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static const uint32_t DEVICE_EXTENSION_COUNT = (uint32_t)(sizeof(DEVICE_EXTENSION_LIST) / sizeof(DEVICE_EXTENSION_LIST[0]));

#ifndef CLAMP
#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#endif

typedef struct{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
}QueueFamilyIndices;

typedef struct{
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t formats_count;
    VkPresentModeKHR *presentModes;
    uint32_t presentModes_count;
}SwapChainSupportDetails;

bool is_indices_complete(const QueueFamilyIndices *indices);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
bool checkDeviceExtensionSupport(VkPhysicalDevice device);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
void cleanupSwapChainSupportDetails(SwapChainSupportDetails *details);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *availableFormats, uint32_t formatCount);
VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR *availablePresentModes, uint32_t presentModeCount);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR *capabilities, Window *window);
bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
uint8_t *readFile(const char *filename, size_t *pCodeSize);
VkShaderModule createShaderModule(VkDevice device, const uint8_t *code, size_t codeSize);
uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

#endif