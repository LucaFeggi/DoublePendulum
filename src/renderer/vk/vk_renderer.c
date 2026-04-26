#include "vk_renderer.h"

#include "vertex.h"
#include "vk_utils.h"

#include "../config.h"

#include <string.h>

#include <SDL_vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2
#define PENDULUM_VERTICES TOTAL_PENDULUMS * 4

static inline bool createInstance(VkInstance *instance, const Window *window);
static inline bool createSurface(const VkInstance instance, const Window *window, VkSurfaceKHR *surface);
static inline bool pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice *physicalDevice);
static inline bool createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice *device, VkQueue *graphicsQueue, VkQueue *presentQueue);
static inline bool createSwapChain(VkPhysicalDevice physicalDevice,
                                   VkSurfaceKHR surface,
                                   VkDevice device,
                                   VkSwapchainKHR *swapChain,
                                   VkImage **swapChainImages,
                                   uint32_t *swapChainImagesCount,
                                   VkFormat *swapChainImageFormat,
                                   VkExtent2D *swapChainExtent,
                                   Window *window);
static inline bool createImageViews(VkDevice device,
                                    VkImage *swapChainImages,
                                    uint32_t swapChainImagesCount,
                                    VkFormat swapChainImageFormat,
                                    VkImageView **swapChainImageViews_out);
static inline bool createRenderPass(VkDevice device,
                                    VkFormat swapChainImageFormat,
                                    VkRenderPass *renderPass_out);
static inline bool createFrameBuffers(VkDevice device,
                                      uint32_t imageCount,
                                      VkImageView *imageViews,
                                      VkRenderPass renderPass,
                                      VkExtent2D extent,
                                      VkFramebuffer **framebuffers_out);
static inline bool createGraphicsPipeline(VkDevice device,
                                          VkRenderPass renderPass,
                                          VkPipelineLayout *pipelineLayout_out,
                                          VkPipeline *pipeline_out);
static inline bool createCommandPool(VkDevice device,
                                     VkPhysicalDevice physicalDevice,
                                     VkSurfaceKHR surface,
                                     VkCommandPool *commandPool_out);
static inline bool createBuffer(VkDevice device,
                                VkPhysicalDevice physicalDevice,
                                VkDeviceSize size,
                                VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkBuffer *buffer_out,
                                VkDeviceMemory *bufferMemory_out);
static inline bool copyBuffer(VkContext *context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
static inline bool createVertexBuffer(VkContext *context);

static inline bool createCommandBuffers(VkContext *context);
static inline bool createSyncObjects(VkContext *context);
static inline void recordCommandBuffer(VkContext *context, VkCommandBuffer commandBuffer, uint32_t imageIndex, const uint32_t *indices, size_t indexCount);
static inline void cleanupSwapChain(VkContext *context);
static inline void recreateSwapChain(VkContext *context, Window *window);


static inline bool createInstance(VkInstance *instance, const Window *window) {
    if(!instance || !window || !window->ptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createInstance: Invalid arguments");
        return false;
    }

    VkApplicationInfo appInfo = { 0 };
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Double pendulum";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    unsigned int sdlExtensionCount = 0;
    if(!SDL_Vulkan_GetInstanceExtensions(window->ptr, &sdlExtensionCount, NULL)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Vulkan_GetInstanceExtensions failed (count): %s", SDL_GetError());
        return false;
    }

    const char **sdlExtensions = malloc(sizeof(const char *) * sdlExtensionCount);
    if(!sdlExtensions) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate memory for Vulkan extensions");
        return false;
    }

    if(!SDL_Vulkan_GetInstanceExtensions(window->ptr, &sdlExtensionCount, sdlExtensions)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Vulkan_GetInstanceExtensions failed (list): %s", SDL_GetError());
        free(sdlExtensions);
        return false;
    }

    VkInstanceCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = sdlExtensionCount;
    createInfo.ppEnabledExtensionNames = sdlExtensions;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = NULL;

    VkResult result = vkCreateInstance(&createInfo, NULL, instance);
    free(sdlExtensions);

    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "vkCreateInstance failed with code %d", result);
        return false;
    }

    return true;
}

static inline bool createSurface(const VkInstance instance, const Window *window, VkSurfaceKHR *surface) {
    if(!instance || !window || !window->ptr || !surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createSurface: Invalid arguments");
        return false;
    }

    if(!SDL_Vulkan_CreateSurface(window->ptr, instance, surface)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create Vulkan surface: %s", SDL_GetError());
        return false;
    }

    return true;
}

bool pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice *physicalDevice) {
    if(!instance || !surface || !physicalDevice) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "pickPhysicalDevice: invalid arguments");
        return false;
    }

    *physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if(result != VK_SUCCESS || deviceCount == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No Vulkan-capable GPUs found!");
        return false;
    }

    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    if(!devices) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Out of memory enumerating devices");
        return false;
    }

    result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "vkEnumeratePhysicalDevices failed: %d", result);
        free(devices);
        return false;
    }

    for(uint32_t i = 0; i < deviceCount; i++) {
        if(isDeviceSuitable(devices[i], surface)) {
            *physicalDevice = devices[i];
            break;
        }
    }

    free(devices);

    if(*physicalDevice == VK_NULL_HANDLE) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to find a suitable GPU!");
        return false;
    }

    return true;
}

static inline bool createLogicalDevice(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkDevice *device,
    VkQueue *graphicsQueue,
    VkQueue *presentQueue
) {
    if(physicalDevice == VK_NULL_HANDLE || surface == VK_NULL_HANDLE ||
       device == NULL || graphicsQueue == NULL || presentQueue == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createLogicalDevice: Invalid arguments");
        return false;
    }

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    if(!is_indices_complete(&indices)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createLogicalDevice: Incomplete queue family indices");
        return false;
    }

    uint32_t uniqueQueueFamilies[2];
    int count = 0;
    uniqueQueueFamilies[count++] = indices.graphicsFamily;
    if(indices.graphicsFamily != indices.presentFamily) {
        uniqueQueueFamilies[count++] = indices.presentFamily;
    }

    VkDeviceQueueCreateInfo *queueCreateInfos = malloc(sizeof(VkDeviceQueueCreateInfo) * count);
    if(!queueCreateInfos) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate memory for queue creation info");
        return false;
    }

    float queuePriority = 1.0f;
    for(int i = 0; i < count; ++i) {
        // zero the struct then set fields (defensive)
        memset(&queueCreateInfos[i], 0, sizeof(VkDeviceQueueCreateInfo));
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].pNext = NULL;
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    VkPhysicalDeviceFeatures supportedFeatures = { 0 };
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    VkPhysicalDeviceFeatures enabledFeatures = { 0 };
    if(supportedFeatures.fillModeNonSolid == VK_TRUE) {
        enabledFeatures.fillModeNonSolid = VK_TRUE;
    }
    else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "Physical device does not support fillModeNonSolid. Wireframe pipelines may fail.");
    }

    // --- IMPORTANT: Provide an array of extension name pointers ---
    // Replace DEVICE_EXTENSION_LIST and DEVICE_EXTENSION_COUNT with your definitions.
    // Example:
    // static const char *DEVICE_EXTENSION_LIST[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    // #define DEVICE_EXTENSION_COUNT (sizeof(DEVICE_EXTENSION_LIST)/sizeof(DEVICE_EXTENSION_LIST[0]))
    extern const char *DEVICE_EXTENSION_LIST[]; // declare (or include from header)
    extern const uint32_t DEVICE_EXTENSION_COUNT; // declare (or include from header)

    VkDeviceCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.queueCreateInfoCount = (uint32_t)count;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.pEnabledFeatures = &enabledFeatures;

    // Use the array pointer (not a single string pointer)
    createInfo.enabledExtensionCount = (uint32_t)DEVICE_EXTENSION_COUNT;
    createInfo.ppEnabledExtensionNames = DEVICE_EXTENSION_COUNT > 0 ? DEVICE_EXTENSION_LIST : NULL;

    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = NULL;

    VkResult result = vkCreateDevice(physicalDevice, &createInfo, NULL, device);
    free(queueCreateInfos);

    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create logical device! Vulkan Error: %d", result);
        *device = VK_NULL_HANDLE;
        return false;
    }

    // Retrieve queues
    vkGetDeviceQueue(*device, indices.graphicsFamily, 0, graphicsQueue);
    if(indices.graphicsFamily != indices.presentFamily) {
        vkGetDeviceQueue(*device, indices.presentFamily, 0, presentQueue);
    }
    else {
        *presentQueue = *graphicsQueue;
    }

    return true;
}

static inline bool createSwapChain(VkPhysicalDevice physicalDevice,
                                   VkSurfaceKHR surface,
                                   VkDevice device,
                                   VkSwapchainKHR *swapChain,
                                   VkImage **swapChainImages,            
                                   uint32_t *swapChainImagesCount,
                                   VkFormat *swapChainImageFormat,
                                   VkExtent2D *swapChainExtent,
                                   Window *window) {

    if(!window || !swapChainImages) { // Check the new double-pointer as well
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createSwapChain: Invalid arguments");
        return false;
    }

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    VkExtent2D extent = chooseSwapExtent(&swapChainSupport.capabilities, window);
    if(extent.width == 0 || extent.height == 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Skipping swapchain creation because extent is 0x0");
        cleanupSwapChainSupportDetails(&swapChainSupport);
        return false;
    }

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formats_count);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModes_count);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    uint32_t *queueFamilyIndices = NULL;

    if(indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;

        queueFamilyIndices = malloc(sizeof(uint32_t) * 2);
        if(!queueFamilyIndices) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate memory for queue family indices");
            cleanupSwapChainSupportDetails(&swapChainSupport);
            return false;
        }

        queueFamilyIndices[0] = indices.graphicsFamily;
        queueFamilyIndices[1] = indices.presentFamily;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(device, &createInfo, NULL, swapChain);

    if(queueFamilyIndices) free(queueFamilyIndices);
    cleanupSwapChainSupportDetails(&swapChainSupport);

    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create swap chain: %d", result);
        return false;
    }

    // Retrieve swap chain images
    // First call to get the exact final image count
    result = vkGetSwapchainImagesKHR(device, *swapChain, &imageCount, NULL);
    if(result != VK_SUCCESS || imageCount == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get swap chain image count: %d", result);
        return false;
    }

    *swapChainImagesCount = imageCount;

    // FIX: Allocate memory and update the caller's pointer (*swapChainImages)
    *swapChainImages = malloc(sizeof(VkImage) * imageCount);
    if(!*swapChainImages) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate memory for swap chain images");
        return false;
    }

    // Second call to populate the array
    // FIX: Dereference swapChain and use the newly allocated pointer *swapChainImages
    result = vkGetSwapchainImagesKHR(device, *swapChain, &imageCount, *swapChainImages);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get swap chain images: %d", result);
        free(*swapChainImages);
        *swapChainImages = NULL;
        return false;
    }

    *swapChainImageFormat = surfaceFormat.format;
    *swapChainExtent = extent;

    return true;
}

static inline bool createImageViews(VkDevice device,
                                    VkImage *swapChainImages,
                                    uint32_t swapChainImagesCount,
                                    VkFormat swapChainImageFormat,
                                    VkImageView **swapChainImageViews_out) { // Output parameter for the array

    // Basic validity checks on input arguments
    if(!device || swapChainImagesCount == 0 || !swapChainImages || !swapChainImageViews_out) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createImageViews: Invalid arguments");
        return false;
    }

    // Allocate memory for the array of VkImageView handles.
    // The result is stored in the caller's pointer (*swapChainImageViews_out).
    *swapChainImageViews_out = malloc(sizeof(VkImageView) * swapChainImagesCount);
    if(!*swapChainImageViews_out) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate memory for swap chain image views");
        return false;
    }

    VkImageView *swapChainImageViews = *swapChainImageViews_out;

    // Loop through each swap chain image and create a corresponding image view
    for(uint32_t i = 0; i < swapChainImagesCount; ++i) {
        VkImageViewCreateInfo createInfo = { 0 };
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = NULL;
        createInfo.flags = 0;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;

        // Standard swizzling for color image
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // Standard subresource range for a single-layer, single-mip color attachment
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create image view at index %u", i);

            // On failure, clean up all image views created so far
            for(uint32_t j = 0; j < i; ++j) {
                vkDestroyImageView(device, swapChainImageViews[j], NULL);
            }
            free(swapChainImageViews);
            // Crucial: Set the caller's pointer to NULL on failure
            *swapChainImageViews_out = NULL;

            return false;
        }
    }

    return true;
}

static inline bool createRenderPass(VkDevice device,
                                    VkFormat swapChainImageFormat,
                                    VkRenderPass *renderPass_out) { // Output parameter for VkRenderPass handle

    // Validity check
    if(!device || !renderPass_out) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createRenderPass: Invalid arguments");
        return false;
    }

    // Color Attachment Description
    VkAttachmentDescription colorAttachment = { 0 };
    colorAttachment.format = swapChainImageFormat; // Use passed format
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear the framebuffer before drawing
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store the result for presentation
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // We don't care about previous contents
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Ready for presentation on the screen

    // Color Attachment Reference
    VkAttachmentReference colorAttachmentRef = { 0 };
    colorAttachmentRef.attachment = 0; // Index into the pAttachments array (which is colorAttachment)
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass Description
    VkSubpassDescription subpass = { 0 };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // Reference to our color attachment

    // Subpass Dependency
    // Ensures the render pass waits for the swap chain to finish reading the image before using it.
    VkSubpassDependency dependency = { 0 };
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Dependency before the render pass starts
    dependency.dstSubpass = 0; // Target is our first and only subpass
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Render Pass Creation Info
    VkRenderPassCreateInfo renderPassInfo = { 0 };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // Create the render pass and store it in the output pointer
    VkResult result = vkCreateRenderPass(device, &renderPassInfo, NULL, renderPass_out);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create render pass! Vulkan Error: %d", result);
        return false;
    }

    return true;
}

static inline bool createFrameBuffers(VkDevice device,
                                      uint32_t imageCount,
                                      VkImageView *imageViews,
                                      VkRenderPass renderPass,
                                      VkExtent2D extent,
                                      VkFramebuffer **framebuffers_out) {

    // 1. Validity Checks
    if(!device || imageCount == 0 || !imageViews || renderPass == VK_NULL_HANDLE || !framebuffers_out) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createFrameBuffers: Invalid arguments or missing resources");
        return false;
    }

    // 2. Allocate memory for the array of VkFramebuffer handles
    *framebuffers_out = malloc(sizeof(VkFramebuffer) * imageCount);
    if(!*framebuffers_out) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate memory for framebuffers");
        return false;
    }

    VkFramebuffer *framebuffers = *framebuffers_out;

    // 3. Loop and create framebuffers
    for(uint32_t i = 0; i < imageCount; ++i) {
        // A simple render pass has one attachment (the color image view)
        VkImageView attachments[] = { imageViews[i] };

        VkFramebufferCreateInfo framebufferInfo = { 0 };
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass; // Link to the render pass
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments; // Attach the current image view
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if(vkCreateFramebuffer(device, &framebufferInfo, NULL, &framebuffers[i]) != VK_SUCCESS) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create framebuffer %u", i);

            // Cleanup: Destroy all framebuffers created successfully up to this point
            for(uint32_t j = 0; j < i; ++j) {
                vkDestroyFramebuffer(device, framebuffers[j], NULL);
            }
            free(framebuffers);
            *framebuffers_out = NULL; // Crucial: Clear the caller's pointer on failure
            return false;
        }
    }

    return true;
}

static inline bool createGraphicsPipeline(VkDevice device,
                                          VkRenderPass renderPass,
                                          VkPipelineLayout *pipelineLayout_out, // Output for PipelineContext.layout
                                          VkPipeline *pipeline_out) {          // Output for PipelineContext.pipeline

    // 0. Validity Check
    if(!device || renderPass == VK_NULL_HANDLE || !pipelineLayout_out || !pipeline_out) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createGraphicsPipeline: Invalid arguments or missing Vulkan handles");
        return false;
    }

    // 1. Read shader code (Assuming readFile is a function that loads SPIR-V files)
    size_t vertShaderCodeSize = 0;
    uint8_t *vertShaderCode = readFile("shaders/vert.spv", &vertShaderCodeSize);
    if(!vertShaderCode) return false;

    size_t fragShaderCodeSize = 0;
    uint8_t *fragShaderCode = readFile("shaders/frag.spv", &fragShaderCodeSize);
    if(!fragShaderCode) {
        free(vertShaderCode);
        return false;
    }

    // 2. Create shader modules (Assuming createShaderModule is a function that wraps VkShaderModuleCreateInfo)
    VkShaderModule vertShaderModule = createShaderModule(device, vertShaderCode, vertShaderCodeSize);
    VkShaderModule fragShaderModule = createShaderModule(device, fragShaderCode, fragShaderCodeSize);

    free(vertShaderCode);
    free(fragShaderCode);

    if(vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE) {
        if(vertShaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(device, vertShaderModule, NULL);
        if(fragShaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(device, fragShaderModule, NULL);
        return false;
    }

    // 3. Shader stages
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main"
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main"
    };
    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // 4. Fixed-function stages

    // Vertex Input (Uses helper functions for attribute and binding descriptions)
    VkVertexInputBindingDescription bindingDescription = vertex_get_binding_description();
    VkVertexInputAttributeDescription attributeDescriptions[2];
    get_vertex_attribute_descriptions(attributeDescriptions, 2);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = attributeDescriptions
    };

    // Input Assembly (Uses VK_PRIMITIVE_TOPOLOGY_LINE_STRIP as requested)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    // Viewport and Scissor (Dynamic)
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
        .pViewports = NULL,
        .pScissors = NULL
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    const VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates
    };

    // 5. Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
        // Assuming no descriptor sets or push constants are needed for this simple pipeline
    };

    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, pipelineLayout_out);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to create pipeline layout! Vulkan Error: %d", result);
        vkDestroyShaderModule(device, fragShaderModule, NULL);
        vkDestroyShaderModule(device, vertShaderModule, NULL);
        return false;
    }

    // 6. Graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = *pipelineLayout_out, // Use the created layout handle
        .renderPass = renderPass,      // Use the passed render pass handle
        .subpass = 0
    };

    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, pipeline_out) != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to create graphics pipeline!");
        // Cleanup resources before returning failure
        vkDestroyPipelineLayout(device, *pipelineLayout_out, NULL);
        vkDestroyShaderModule(device, fragShaderModule, NULL);
        vkDestroyShaderModule(device, vertShaderModule, NULL);
        return false;
    }

    // 7. Cleanup shader modules
    vkDestroyShaderModule(device, fragShaderModule, NULL);
    vkDestroyShaderModule(device, vertShaderModule, NULL);

    return true;
}

static inline bool createCommandPool(VkDevice device,
                                     VkPhysicalDevice physicalDevice,
                                     VkSurfaceKHR surface,
                                     VkCommandPool *commandPool_out) { // Output parameter for VkCommandPool handle

    // NOTE: Assumes QueueFamilyIndices, QUEUE_FAMILY_INDEX_INVALID, 
    //       and findQueueFamilies are defined elsewhere.

    // 1. Validity Checks
    if(!device || physicalDevice == VK_NULL_HANDLE || surface == VK_NULL_HANDLE || !commandPool_out) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createCommandPool: Invalid arguments or Vulkan handles");
        return false;
    }

    // 2. Get the Graphics Queue Family Index
    // This requires both the physical device and the surface.
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);
    if(queueFamilyIndices.graphicsFamily == QUEUE_FAMILY_INDEX_INVALID) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Graphics queue family index is invalid, cannot create command pool.");
        return false;
    }

    // 3. Setup Command Pool Creation Info
    VkCommandPoolCreateInfo poolInfo = { 0 };
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT is necessary if you reuse command buffers by resetting them individually.
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    // 4. Create the Command Pool
    VkResult result = vkCreateCommandPool(device, &poolInfo, NULL, commandPool_out);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to create command pool! Vulkan Error: %d", result);
        *commandPool_out = VK_NULL_HANDLE; // Ensure output is NULL on failure
        return false;
    }

    return true;
}

static inline bool createBuffer(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *buffer_out,         // Output for the VkBuffer handle
    VkDeviceMemory *bufferMemory_out // Output for the VkDeviceMemory handle
) {
    // NOTE: Assumes findMemoryType is defined elsewhere.

    // 1. Validity Checks
    if(!device || physicalDevice == VK_NULL_HANDLE || !buffer_out || !bufferMemory_out) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createBuffer called with invalid Vulkan handles or output pointers.");
        if(buffer_out) *buffer_out = VK_NULL_HANDLE;
        if(bufferMemory_out) *bufferMemory_out = VK_NULL_HANDLE;
        return false;
    }

    // Initialize output pointers to NULL_HANDLE in case of early exit
    *buffer_out = VK_NULL_HANDLE;
    *bufferMemory_out = VK_NULL_HANDLE;

    // 2. Create Buffer
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        // Since queue family indices are usually the same for graphics and transfer, 
        // we default to exclusive mode for simplicity, which is correct here.
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if(vkCreateBuffer(device, &bufferInfo, NULL, buffer_out) != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create buffer.");
        return false;
    }

    // 3. Get Memory Requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer_out, &memRequirements);

    // 4. Allocate Memory
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        // Use physicalDevice to find the correct memory type
        .memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties)
    };

    if(vkAllocateMemory(device, &allocInfo, NULL, bufferMemory_out) != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate buffer memory.");
        // Clean up the created buffer before returning
        vkDestroyBuffer(device, *buffer_out, NULL);
        *buffer_out = VK_NULL_HANDLE;
        return false;
    }

    // 5. Bind Buffer to Memory
    if(vkBindBufferMemory(device, *buffer_out, *bufferMemory_out, 0) != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to bind buffer memory.");

        // Clean up both the allocated memory and the created buffer
        if(*bufferMemory_out != VK_NULL_HANDLE) {
            vkFreeMemory(device, *bufferMemory_out, NULL);
            *bufferMemory_out = VK_NULL_HANDLE;
        }
        if(*buffer_out != VK_NULL_HANDLE) {
            vkDestroyBuffer(device, *buffer_out, NULL);
            *buffer_out = VK_NULL_HANDLE;
        }
        return false;
    }

    return true;
}

static inline bool copyBuffer(
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue graphicsQueue,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize size
) {
    // 1. Validity Checks
    if(!device || commandPool == VK_NULL_HANDLE || graphicsQueue == VK_NULL_HANDLE || srcBuffer == VK_NULL_HANDLE || dstBuffer == VK_NULL_HANDLE) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "copyBuffer: Invalid arguments or uninitialized Vulkan handles");
        return false;
    }

    // 2. Allocate a temporary command buffer
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool, // Use passed pool
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "copyBuffer: Failed to allocate command buffer! Error %d", result);
        return false;
    }

    // 3. Begin command buffer recording
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = NULL
    };

    result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "copyBuffer: Failed to begin command buffer! Error %d", result);
        // Use passed handles for cleanup
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        return false;
    }

    // 4. Copy buffer region
    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    // 5. End command buffer recording
    result = vkEndCommandBuffer(commandBuffer);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "copyBuffer: Failed to end command buffer! Error %d", result);
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        return false;
    }

    // 6. Submit command buffer
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = NULL,
        .pWaitDstStageMask = NULL,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = NULL
    };

    // Use passed queue
    result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "copyBuffer: Failed to submit command buffer! Error %d", result);
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        return false;
    }

    // 7. Wait for completion (Synchronizing the transfer)
    // Use passed queue
    result = vkQueueWaitIdle(graphicsQueue);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "copyBuffer: Failed to wait for queue idle! Error %d", result);
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        return false;
    }

    // 8. Cleanup
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

    return true;
}

static inline bool createVertexBuffer(VkDevice device,
                                      VkPhysicalDevice physicalDevice,
                                      VkCommandPool commandPool,
                                      VkQueue graphicsQueue,
                                      VkBuffer *vertexBuffer_out,           // Output for PipelineContext.vertexBuffer
                                      VkDeviceMemory *vertexBufferMemory_out) { // Output for PipelineContext.vertexBufferMemory

    // NOTE: Assumes the following variables/functions are defined globally or included:
    //       vertices (array of vertex data), VERTEX_COUNT (number of vertices),
    //       createBuffer (refactored), and copyBuffer (refactored).

    // 1. Validity Checks
    if(!device || physicalDevice == VK_NULL_HANDLE || commandPool == VK_NULL_HANDLE || graphicsQueue == VK_NULL_HANDLE || !vertexBuffer_out || !vertexBufferMemory_out) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createVertexBuffer: Invalid arguments or device handle");
        return false;
    }

    // Initialize output pointers
    *vertexBuffer_out = VK_NULL_HANDLE;
    *vertexBufferMemory_out = VK_NULL_HANDLE;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize bufferSize = sizeof(Vertex) * PENDULUM_VERTICES;

    // --- 1. Create Staging Buffer (Host Visible) ---
    if(!createBuffer(device,
                     physicalDevice,
                     bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &stagingBuffer,
                     &stagingBufferMemory)) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to create staging buffer");
        return false;
    }

    // --- 2. Map and copy vertex data ---
    void *data = NULL;
    VkResult result = vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    if(result != VK_SUCCESS || !data) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to map staging buffer memory. Vulkan Error: %d", result);
        vkDestroyBuffer(device, stagingBuffer, NULL);
        vkFreeMemory(device, stagingBufferMemory, NULL);
        return false;
    }

    memcpy(data, vertices, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // --- 3. Create Device-Local Vertex Buffer (The actual buffer) ---
    if(!createBuffer(device,
                     physicalDevice,
                     bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     vertexBuffer_out,      // Output
                     vertexBufferMemory_out)) { // Output
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to create device-local vertex buffer");
        // Cleanup staging buffer before returning
        vkDestroyBuffer(device, stagingBuffer, NULL);
        vkFreeMemory(device, stagingBufferMemory, NULL);
        return false;
    }

    // --- 4. Copy data from staging buffer to vertex buffer ---
    if(!copyBuffer(device,
                   commandPool,
                   graphicsQueue,
                   stagingBuffer,
                   *vertexBuffer_out, // Use the newly created vertex buffer handle
                   bufferSize)) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to copy vertex data to device-local buffer");

        // Full cleanup on failure
        vkDestroyBuffer(device, *vertexBuffer_out, NULL);
        vkFreeMemory(device, *vertexBufferMemory_out, NULL);
        *vertexBuffer_out = VK_NULL_HANDLE;
        *vertexBufferMemory_out = VK_NULL_HANDLE;
        vkDestroyBuffer(device, stagingBuffer, NULL);
        vkFreeMemory(device, stagingBufferMemory, NULL);
        return false;
    }

    // --- 5. Clean up staging buffer ---
    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    return true;
}

static inline bool createCommandBuffers(VkContext *context) {
    if(!context || context->device == VK_NULL_HANDLE || context->commandPool == VK_NULL_HANDLE) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "createCommandBuffers: Invalid context or uninitialized Vulkan handles");
        return false;
    }

    // 1. Allocate memory for the command buffers array
    context->maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
    context->commandBuffers = (VkCommandBuffer *)calloc(context->maxFramesInFlight, sizeof(VkCommandBuffer));
    if(!context->commandBuffers) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "createCommandBuffers: Failed to allocate memory for command buffers");
        return false;
    }

    // 2. Setup Command Buffer Allocation Info
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = context->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = context->maxFramesInFlight
    };

    // 3. Allocate the command buffers
    VkResult result = vkAllocateCommandBuffers(context->device, &allocInfo, context->commandBuffers);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "createCommandBuffers: Failed to allocate command buffers! Vulkan Error: %d", result);
        free(context->commandBuffers);
        context->commandBuffers = NULL;
        return false;
    }

    return true;
}

static inline bool createSyncObjects(VkContext *context) {
    if(!context || context->device == VK_NULL_HANDLE) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createSyncObjects: Invalid context or uninitialized device");
        return false;
    }

    context->maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;

    // 1. Allocate memory for arrays
    context->imageAvailableSemaphores = (VkSemaphore *)calloc(context->maxFramesInFlight, sizeof(VkSemaphore));
    context->renderFinishedSemaphores = (VkSemaphore *)calloc(context->maxFramesInFlight, sizeof(VkSemaphore));
    context->inFlightFences = (VkFence *)calloc(context->maxFramesInFlight, sizeof(VkFence));

    if(!context->imageAvailableSemaphores || !context->renderFinishedSemaphores || !context->inFlightFences) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createSyncObjects: Failed to allocate memory for sync object arrays");
        free(context->imageAvailableSemaphores);
        free(context->renderFinishedSemaphores);
        free(context->inFlightFences);
        context->imageAvailableSemaphores = NULL;
        context->renderFinishedSemaphores = NULL;
        context->inFlightFences = NULL;
        return false;
    }

    // 2. Setup Create Infos
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    // 3. Create objects in a loop
    for(uint32_t i = 0; i < context->maxFramesInFlight; i++) {
        VkResult res_sem1 = vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &context->imageAvailableSemaphores[i]);
        VkResult res_sem2 = vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &context->renderFinishedSemaphores[i]);
        VkResult res_fence = vkCreateFence(context->device, &fenceInfo, NULL, &context->inFlightFences[i]);

        if(res_sem1 != VK_SUCCESS || res_sem2 != VK_SUCCESS || res_fence != VK_SUCCESS) {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                         "createSyncObjects: Failed to create sync objects for frame %u (Sem1=%d, Sem2=%d, Fence=%d)",
                         i, res_sem1, res_sem2, res_fence);

            // Clean up previously created semaphores/fences
            for(uint32_t j = 0; j <= i; j++) {
                if(context->imageAvailableSemaphores[j] != VK_NULL_HANDLE)
                    vkDestroySemaphore(context->device, context->imageAvailableSemaphores[j], NULL);
                if(context->renderFinishedSemaphores[j] != VK_NULL_HANDLE)
                    vkDestroySemaphore(context->device, context->renderFinishedSemaphores[j], NULL);
                if(context->inFlightFences[j] != VK_NULL_HANDLE)
                    vkDestroyFence(context->device, context->inFlightFences[j], NULL);
            }

            free(context->imageAvailableSemaphores);
            free(context->renderFinishedSemaphores);
            free(context->inFlightFences);
            context->imageAvailableSemaphores = NULL;
            context->renderFinishedSemaphores = NULL;
            context->inFlightFences = NULL;

            return false;
        }
    }

    context->currentFrame = 0;
    return true;
}

static inline void recordCommandBuffer(VkContext *context, VkCommandBuffer commandBuffer, uint32_t imageIndex, const uint32_t *indices, size_t indexCount) {
    if(!context || commandBuffer == VK_NULL_HANDLE || imageIndex >= context->swapChainImagesCount ||
       !indices || indexCount == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "recordCommandBuffer: Invalid parameters or context");
        return;
    }

    // 1. Begin Command Buffer Recording
    VkCommandBufferBeginInfo beginInfo = { 0 };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to begin recording command buffer! Vulkan Error: %d", result);
        return;
    }

    // 2. Begin Render Pass
    VkRenderPassBeginInfo renderPassInfo = { 0 };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = context->renderPass;
    renderPassInfo.framebuffer = context->swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = context->swapChainExtent;

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // 3. Bind graphics pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->graphicsPipeline);

    // 4. Set viewport
    float W = (float)context->swapChainExtent.width;
    float H = (float)context->swapChainExtent.height;
    float d = W < H ? W / 5.0f * 4.0f : H / 5.0f * 4.0f;    // Setting max rend length
    VkViewport viewport = { 0 };    // Calculate the top-left corner (x, y) to center the square
    viewport.x = (W - d) / 2.0f;    // x = (Total Width - Square Side) / 2
    viewport.y = (H - d) / 2.0f;    // y = (Total Height - Square Side) / 2
    viewport.width = d;
    viewport.height = d;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    // 5. Set scissor
    VkRect2D scissor = { 0 };
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = context->swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // 6. Bind vertex buffer
    VkBuffer vertexBuffers[] = { context->vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    // 7. Bind index buffer
    vkCmdBindIndexBuffer(commandBuffer, context->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    // 8. Draw indexed
    vkCmdDrawIndexed(commandBuffer, (uint32_t)indexCount, 1, 0, 0, 0);

    // 9. End render pass
    vkCmdEndRenderPass(commandBuffer);

    // 10. End command buffer
    result = vkEndCommandBuffer(commandBuffer);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to record command buffer! Vulkan Error: %d", result);
    }
}

static inline void cleanupSwapChain(VkContext *context) {
    if(!context || context->device == VK_NULL_HANDLE) return;

    if(context->swapChainFramebuffers) {
        for(uint32_t i = 0; i < context->swapChainImagesCount; ++i)
            if(context->swapChainFramebuffers[i] != VK_NULL_HANDLE)
                vkDestroyFramebuffer(context->device, context->swapChainFramebuffers[i], NULL);
        free(context->swapChainFramebuffers);
        context->swapChainFramebuffers = NULL;
    }

    if(context->swapChainImageViews) {
        for(uint32_t i = 0; i < context->swapChainImagesCount; ++i)
            if(context->swapChainImageViews[i] != VK_NULL_HANDLE)
                vkDestroyImageView(context->device, context->swapChainImageViews[i], NULL);
        free(context->swapChainImageViews);
        context->swapChainImageViews = NULL;
    }

    if(context->swapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(context->device, context->swapChain, NULL);
        context->swapChain = VK_NULL_HANDLE;
    }

    free(context->swapChainImages);
    context->swapChainImages = NULL;
    context->swapChainImagesCount = 0;
}


static inline void recreateSwapChain(VkContext *context, Window *window) {
    if(!context || !window || context->device == VK_NULL_HANDLE) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "vk_context_recreate_swap_chain: Invalid context or window");
        return;
    }

    int width = 0, height = 0;
    SDL_Vulkan_GetDrawableSize(window->ptr, &width, &height);
    if(width == 0 || height == 0) {
        // Window is minimized or not drawable yet
        return;
    }

    vkDeviceWaitIdle(context->device);

    cleanupSwapChain(context);

    createSwapChain(context, window);
    createImageViews(context);
    createFrameBuffers(context);
}

bool vk_context_init(VkContext *context, Window *window) {
    if(!createInstance(&context->instance, window)) goto fail_instance;
    if(!createSurface(context->instance, window, &context->surface)) goto fail_surface;
    if(!pickPhysicalDevice(context->instance, context->surface, &context->physicalDevice)) goto fail_physical;
    if(!createLogicalDevice(context->physicalDevice, context->surface, &context->device, &context->graphicsQueue, &context->presentQueue)) goto fail_device;
    if(!createSwapChain(context, window)) goto fail_swapchain;
    if(!createImageViews(context)) goto fail_imageviews;
    if(!createRenderPass(context)) goto fail_renderpass;
    if(!createFrameBuffers(context)) goto fail_framebuffers;
    if(!createGraphicsPipeline(context)) goto fail_pipeline;
    if(!createCommandPool(context)) goto fail_cmdpool;
    if(!createVertexBuffer(context)) goto fail_vertexbuffer;
    if(!createIndexBuffer(context)) goto fail_indexbuffer;
    if(!createCommandBuffers(context)) goto fail_cmdbuffers;
    if(!createSyncObjects(context)) goto fail_sync;

    return true;

    // --- Cleanup chain ---
fail_sync:
    // command buffers freed with command pool
fail_cmdbuffers:
    // free index buffer + memory
fail_indexbuffer:
    if(context->indexBuffer) vkDestroyBuffer(context->device, context->indexBuffer, NULL);
    if(context->indexBufferMemory) vkFreeMemory(context->device, context->indexBufferMemory, NULL);
fail_vertexbuffer:
    if(context->vertexBuffer) vkDestroyBuffer(context->device, context->vertexBuffer, NULL);
    if(context->vertexBufferMemory) vkFreeMemory(context->device, context->vertexBufferMemory, NULL);
fail_cmdpool:
    if(context->commandPool) vkDestroyCommandPool(context->device, context->commandPool, NULL);
fail_pipeline:
    if(context->graphicsPipeline) vkDestroyPipeline(context->device, context->graphicsPipeline, NULL);
    if(context->pipelineLayout) vkDestroyPipelineLayout(context->device, context->pipelineLayout, NULL);
fail_renderpass:
    if(context->renderPass) vkDestroyRenderPass(context->device, context->renderPass, NULL);
fail_framebuffers:
    if(context->swapChainFramebuffers) {
        for(uint32_t i = 0; i < context->swapChainImagesCount; i++) {
            vkDestroyFramebuffer(context->device, context->swapChainFramebuffers[i], NULL);
        }
        free(context->swapChainFramebuffers);
    }
fail_imageviews:
    if(context->swapChainImageViews) {
        for(uint32_t i = 0; i < context->swapChainImagesCount; i++) {
            vkDestroyImageView(context->device, context->swapChainImageViews[i], NULL);
        }
        free(context->swapChainImageViews);
    }
fail_swapchain:
    if(context->swapChain) vkDestroySwapchainKHR(context->device, context->swapChain, NULL);
fail_device:
    if(context->device) vkDestroyDevice(context->device, NULL);
fail_physical:
    if(context->surface) vkDestroySurfaceKHR(context->instance, context->surface, NULL);
fail_surface:
    if(context->instance) vkDestroyInstance(context->instance, NULL);
fail_instance:
    return false;
}

void vk_context_free(VkContext *context) {
    if(!context) return;

    if(context->device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(context->device);

    cleanupSwapChain(context);

    if(context->indexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(context->device, context->indexBuffer, NULL);
        context->indexBuffer = VK_NULL_HANDLE;
    }
    if(context->indexBuffer != VK_NULL_HANDLE) {
        vkFreeMemory(context->device, context->indexBufferMemory, NULL);
        context->indexBufferMemory = VK_NULL_HANDLE;
    }

    if(context->vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(context->device, context->vertexBuffer, NULL);
        context->vertexBuffer = VK_NULL_HANDLE;
    }
    if(context->vertexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(context->device, context->vertexBufferMemory, NULL);
        context->vertexBufferMemory = VK_NULL_HANDLE;
    }

    if(context->graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(context->device, context->graphicsPipeline, NULL);
        context->graphicsPipeline = VK_NULL_HANDLE;
    }
    if(context->pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(context->device, context->pipelineLayout, NULL);
        context->pipelineLayout = VK_NULL_HANDLE;
    }

    if(context->renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(context->device, context->renderPass, NULL);
        context->renderPass = VK_NULL_HANDLE;
    }

    if(context->maxFramesInFlight > 0) {
        for(uint32_t i = 0; i < context->maxFramesInFlight; i++) {
            if(context->renderFinishedSemaphores && context->renderFinishedSemaphores[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(context->device, context->renderFinishedSemaphores[i], NULL);
            if(context->imageAvailableSemaphores && context->imageAvailableSemaphores[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(context->device, context->imageAvailableSemaphores[i], NULL);
            if(context->inFlightFences && context->inFlightFences[i] != VK_NULL_HANDLE)
                vkDestroyFence(context->device, context->inFlightFences[i], NULL);
        }
        free(context->renderFinishedSemaphores);
        free(context->imageAvailableSemaphores);
        free(context->inFlightFences);
        context->renderFinishedSemaphores = NULL;
        context->imageAvailableSemaphores = NULL;
        context->inFlightFences = NULL;
    }

    free(context->commandBuffers);
    context->commandBuffers = NULL;

    if(context->commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(context->device, context->commandPool, NULL);
        context->commandPool = VK_NULL_HANDLE;
    }

    if(context->device != VK_NULL_HANDLE) {
        vkDestroyDevice(context->device, NULL);
        context->device = VK_NULL_HANDLE;
    }
    if(context->surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(context->instance, context->surface, NULL);
        context->surface = VK_NULL_HANDLE;
    }
    if(context->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(context->instance, NULL);
        context->instance = VK_NULL_HANDLE;
    }
}

void vk_context_render(VkContext *context, Window *window) {
    if(!context || !window) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "vk_context_render: invalid arguments");
        return;
    }

    VkResult result;
    uint32_t imageIndex;
    uint32_t frame_index = context->currentFrame;

    if(!context->inFlightFences || !context->commandBuffers ||
       !context->imageAvailableSemaphores || !context->renderFinishedSemaphores) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "vk_context_render: per-frame resources not initialized");
        return;
    }

    VkFence currentFence = context->inFlightFences[frame_index];
    VkCommandBuffer currentCommandBuffer = context->commandBuffers[frame_index];
    VkSemaphore currentImageAvailableSemaphore = context->imageAvailableSemaphores[frame_index];
    VkSemaphore currentRenderFinishedSemaphore = context->renderFinishedSemaphores[frame_index];

    // 1. Wait for the fence
    result = vkWaitForFences(context->device, 1, &currentFence, VK_TRUE, UINT64_MAX);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to wait for fence (frame %u). VkResult=%d",
                     frame_index, result);
        return;
    }

    // 2. Acquire an image
    result = vkAcquireNextImageKHR(
        context->device,
        context->swapChain,
        UINT64_MAX,
        currentImageAvailableSemaphore,
        VK_NULL_HANDLE,
        &imageIndex
    );

    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(context, window);
        return;
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to acquire swapchain image! VkResult=%d", result);
        return;
    }

    // 3. Reset fence + command buffer
    if(vkResetFences(context->device, 1, &currentFence) != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to reset fence (frame %u)", frame_index);
        return;
    }
    if(vkResetCommandBuffer(currentCommandBuffer, 0) != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to reset command buffer (frame %u)", frame_index);
        return;
    }

    recordCommandBuffer(context, currentCommandBuffer, imageIndex, indices, INDEX_COUNT);

    // 4. Submit to graphics queue
    VkSemaphore waitSemaphores[] = { currentImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { currentRenderFinishedSemaphore };

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &currentCommandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    result = vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, currentFence);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to submit command buffer (frame %u). VkResult=%d",
                     frame_index, result);
        return;
    }

    // 5. Present
    VkSwapchainKHR swapChains[] = { context->swapChain };

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex
    };

    result = vkQueuePresentKHR(context->presentQueue, &presentInfo);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain(context, window);
        return;
    }
    else if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to present swapchain image! VkResult=%d", result);
        return;
    }

    // 6. Advance to the next frame
    context->currentFrame = (context->currentFrame + 1) % context->maxFramesInFlight;
}


/*
static inline bool createIndexBuffer(VkContext *context);
static inline bool createIndexBuffer(VkContext *context) {
    if(!context || !indices || INDEX_COUNT == 0 || !context->indexBuffer || !context->indexBufferMemory) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid parameters passed to createIndexBuffer.\n");
        return false;
    }

    VkDeviceSize bufferSize = sizeof(uint32_t) * INDEX_COUNT;

    // Staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if(!createBuffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &stagingBuffer, &stagingBufferMemory)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create staging buffer.\n");
        return false;
    }

    // Map memory and copy data
    void *data = NULL;
    VkResult result = vkMapMemory(context->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    if(result != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to map staging buffer memory (VkResult: %d).\n", result);
        vkDestroyBuffer(context->device, stagingBuffer, NULL);
        vkFreeMemory(context->device, stagingBufferMemory, NULL);
        return false;
    }

    memcpy(data, indices, (size_t)bufferSize);
    vkUnmapMemory(context->device, stagingBufferMemory);

    // Create device local index buffer
    if(!createBuffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &context->indexBuffer, &context->indexBufferMemory)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create index buffer.\n");
        vkDestroyBuffer(context->device, stagingBuffer, NULL);
        vkFreeMemory(context->device, stagingBufferMemory, NULL);
        return false;
    }

    // Copy staging buffer to device local buffer
    if(!copyBuffer(context, stagingBuffer, context->indexBuffer, bufferSize)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to copy buffer data.\n");
        vkDestroyBuffer(context->device, stagingBuffer, NULL);
        vkFreeMemory(context->device, stagingBufferMemory, NULL);
        vkDestroyBuffer(context->device, context->indexBuffer, NULL);
        vkFreeMemory(context->device, context->indexBufferMemory, NULL);
        return false;
    }

    // Cleanup staging buffer
    vkDestroyBuffer(context->device, stagingBuffer, NULL);
    vkFreeMemory(context->device, stagingBufferMemory, NULL);

    return true;
}
*/