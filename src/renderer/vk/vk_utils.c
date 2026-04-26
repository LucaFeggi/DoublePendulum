#include "vk_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>    

#include <SDL_vulkan.h>

bool is_indices_complete(const QueueFamilyIndices *indices){
    return (indices->graphicsFamily != QUEUE_FAMILY_INDEX_INVALID) &&
        (indices->presentFamily != QUEUE_FAMILY_INDEX_INVALID);
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;
    indices.graphicsFamily = QUEUE_FAMILY_INDEX_INVALID;
    indices.presentFamily = QUEUE_FAMILY_INDEX_INVALID;
    VkResult result; // Variabile per controllare il risultato della chiamata Vulkan

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    if(queueFamilyCount == 0) {
        return indices;
    }

    // 1. Alloca la memoria per le proprietà della famiglia di code
    VkQueueFamilyProperties *queueFamilies = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    if(queueFamilies == NULL) {
        SDL_Log("ERROR: findQueueFamilies: Failed to allocate memory for queue families.");
        return indices; // Ritorna gli indici non validi in caso di fallimento di malloc
    }

    // 2. Ottiene le proprietà
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    // 3. Itera e cerca le famiglie
    for(uint32_t i = 0; i < queueFamilyCount; ++i) {
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = VK_FALSE;
        // La chiamata vkGetPhysicalDeviceSurfaceSupportKHR non dovrebbe fallire, ma è Vulkan.
        result = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        // Aggiungi un controllo di errore per vkGetPhysicalDeviceSurfaceSupportKHR
        if(result != VK_SUCCESS) {
            SDL_Log("ERROR: findQueueFamilies: vkGetPhysicalDeviceSurfaceSupportKHR failed with result %d.", result);
            free(queueFamilies); // Pulizia in caso di errore Vulkan
            return indices;
        }

        if(presentSupport) {
            indices.presentFamily = i;
        }

        // Assumiamo che is_indices_complete sia disponibile
        if(is_indices_complete(&indices)) {
            break;
        }
    }

    // 4. Libera la memoria allocata prima di uscire (sia in successo che in fallimento logico)
    free(queueFamilies);
    return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    if(device == VK_NULL_HANDLE) {
        SDL_Log("ERROR: checkDeviceExtensionSupport called with VK_NULL_HANDLE device.");
        return false;
    }

    uint32_t extensionCount = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    if(result != VK_SUCCESS) {
        SDL_Log("ERROR: vkEnumerateDeviceExtensionProperties failed with result %d.", result);
        return false;
    }

    if(extensionCount == 0) {
        SDL_Log("No device extensions available.");
        return false;  // If you require VK_KHR_SWAPCHAIN_EXTENSION_NAME, fail here
    }

    VkExtensionProperties *availableExtensions =
        malloc(sizeof(VkExtensionProperties) * extensionCount);
    if(!availableExtensions) {
        SDL_Log("ERROR: Failed to allocate memory for %u extensions.", extensionCount);
        return false;
    }

    result = vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);
    if(result != VK_SUCCESS) {
        SDL_Log("ERROR: vkEnumerateDeviceExtensionProperties (2nd call) failed with result %d.", result);
        free(availableExtensions);
        return false;
    }

    // Check all required extensions
    bool allFound = true;
    for(uint32_t req = 0; req < DEVICE_EXTENSION_COUNT; ++req) {
        bool found = false;
        for(uint32_t i = 0; i < extensionCount; ++i) {
            if(strcmp(availableExtensions[i].extensionName, DEVICE_EXTENSION_LIST[req]) == 0) {
                found = true;
                break;
            }
        }
        if(!found) {
            SDL_Log("Missing required extension: %s", DEVICE_EXTENSION_LIST[req]);
            allFound = false;
        }
    }

    free(availableExtensions);
    return allFound;
}


SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details = { 0 }; // Initialize all members to zero/NULL
    VkResult result;

    // 1. Get capabilities (doesn't return VkResult, but useful to check for valid parameters)
    // Note: This function doesn't return an error code, but will write to the struct.
    // If 'device' or 'surface' is invalid, the behaviour is implementation-defined.
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // --- Retrieve Surface Formats ---

    // 2. Get formats count
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formats_count, NULL);
    if(result != VK_SUCCESS) {
        // Handle error in retrieving count
        // Consider returning a structure that indicates failure if you can't proceed
        // For now, we'll just log and return the partially filled structure.
        SDL_Log("Failed to get physical device surface formats count. VkResult: %d", result);
        return details; // details.formats_count will be 0
    }

    if(details.formats_count != 0) {
        // 3. Allocate memory for formats
        details.formats = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * details.formats_count);

        if(details.formats == NULL) {
            // Handle memory allocation failure
            SDL_Log("Failed to allocate memory for surface formats.");
            details.formats_count = 0; // Ensure count reflects the fact that no formats were stored
            // We can still try to get present modes, so we continue.
        }
        else {
            // 4. Get formats
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formats_count, details.formats);
            if(result != VK_SUCCESS) {
                // Handle error in retrieving the actual data
                SDL_Log("Failed to get physical device surface formats. VkResult: %d", result);
                free(details.formats); // Free allocated memory
                details.formats = NULL;
                details.formats_count = 0;
            }
        }
    }

    // --- Retrieve Present Modes ---

    // 5. Get present modes count
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModes_count, NULL);
    if(result != VK_SUCCESS) {
        SDL_Log("Failed to get physical device surface present modes count. VkResult: %d", result);
        // Note: The structure might already contain formats if that part succeeded.
        return details; // details.presentModes_count will be 0
    }

    if(details.presentModes_count != 0) {
        // 6. Allocate memory for present modes
        details.presentModes = (VkPresentModeKHR *)malloc(sizeof(VkPresentModeKHR) * details.presentModes_count);

        if(details.presentModes == NULL) {
            // Handle memory allocation failure
            SDL_Log("Failed to allocate memory for present modes.");
            details.presentModes_count = 0; // Ensure count reflects the fact that no modes were stored
        }
        else {
            // 7. Get present modes
            result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModes_count, details.presentModes);
            if(result != VK_SUCCESS) {
                // Handle error in retrieving the actual data
                SDL_Log("Failed to get physical device surface present modes. VkResult: %d", result);
                free(details.presentModes); // Free allocated memory
                details.presentModes = NULL;
                details.presentModes_count = 0;
            }
        }
    }

    return details;
}

void cleanupSwapChainSupportDetails(SwapChainSupportDetails *details) {
    // 1. CRITICAL: Check if the 'details' pointer itself is NULL before dereferencing it.
    if(details == NULL) {
        return; // Nothing to clean up if the structure pointer is invalid.
    }

    // 2. Safely free formats and reset pointer/count
    if(details->formats != NULL) {
        free(details->formats);
        details->formats = NULL;
        // Optional: It's good practice to reset the count as well
        // if your struct includes it, to fully clear the structure's state.
        // Assuming your struct has a 'formats_count' field:
        // details->formats_count = 0; 
    }

    // 3. Safely free present modes and reset pointer/count
    if(details->presentModes != NULL) {
        free(details->presentModes);
        details->presentModes = NULL;
        // Assuming your struct has a 'presentModes_count' field:
        // details->presentModes_count = 0;
    }
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *availableFormats, uint32_t formatCount) {
    // Define a zero-initialized (safe) fallback structure using a C compound literal.
    // This is explicitly C11 compliant for zeroing structs.
    const VkSurfaceFormatKHR zero_format = { 0 };

    // Safety check 1: Ensure the pointer is not NULL.
    if(availableFormats == NULL) {
        // In a real application, you should log an error here.
        return zero_format;
    }

    // Safety check 2: Ensure there is at least one format available.
    if(formatCount == 0) {
        // In a real application, you should log an error here.
        return zero_format;
    }

    // Search for the desired format
    for(uint32_t i = 0; i < formatCount; ++i) {
        if(availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
           availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {

            return availableFormats[i];
        }
    }

    // Fallback: If the desired format is not found, return the first one available.
    // This is safe because we checked that formatCount >= 1.
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR *availablePresentModes, uint32_t presentModeCount) {

    // Safety check 1: Ensure the array pointer itself is not NULL.
    // While the subsequent count check helps, checking the pointer first is robust.
    if(availablePresentModes == NULL) {
        // Log an error in a real application. 
        // We can safely fall back to FIFO, as it's mandatory.
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // Safety check 2: Ensure there is at least one mode available.
    // If count is zero, the loop is skipped, and accessing availablePresentModes[i] 
    // is safe as long as we only do it inside the loop.
    // However, if the count is zero, the loop is skipped, and we fall back safely.
    // No explicit check for 'presentModeCount == 0' is strictly necessary before the loop,
    // but we'll leave it in for clarity to emphasize input validation.
    if(presentModeCount == 0) {
        // Log an error in a real application. 
        // Fall back to FIFO, as it's mandatory.
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // Search for the desired present mode: VK_PRESENT_MODE_MAILBOX_KHR
    for(uint32_t i = 0; i < presentModeCount; ++i) {
        // Accessing availablePresentModes[i] is safe because of the loop condition
        // and the check that availablePresentModes is not NULL.
        if(availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    // Mandatory Fallback: 
    // If the preferred mode (MAILBOX) is not found, Vulkan mandates that 
    // VK_PRESENT_MODE_FIFO_KHR must always be available.
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR *capabilities, Window *window) {

    // Define a safe zero-initialized fallback extent using a C compound literal
    const VkExtent2D zero_extent = { .width = 0, .height = 0 };

    // Safety check 1: Validate the 'capabilities' pointer.
    if(capabilities == NULL) {
        // Log an error in a real application. Cannot proceed without capabilities.
        return zero_extent;
    }

    // --- Case 1: Fixed Extent ---
    // Check if the extent is fixed by the surface.
    if(capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    }

    // --- Case 2: Flexible Extent ---

    // Safety check 2: Validate the 'window' pointer.
    if(window == NULL || window->ptr == NULL) {
        // Log an error. Cannot query drawable size without a valid window.
        // In a non-fixed scenario, this is a critical failure.
        return zero_extent;
    }

    int width = 0;
    int height = 0;

    // Use the SDL function to get the current non-zero, actual pixel size.
    // NOTE: SDL_Vulkan_GetDrawableSize may return 0, 0 when minimized or during transitions.
    // It does not return an error code but writes to the pointers.
    SDL_Vulkan_GetDrawableSize(window->ptr, &width, &height);

    // Safety check 3: Ensure dimensions are not negative and convert to uint32_t.
    // Negative values from SDL_Vulkan_GetDrawableSize are highly unlikely but defensive
    // programming dictates checking and clamping.
    uint32_t actualWidth = (uint32_t)(width > 0 ? width : capabilities->minImageExtent.width);
    uint32_t actualHeight = (uint32_t)(height > 0 ? height : capabilities->minImageExtent.height);

    // If SDL reports 0, we clamp it to minImageExtent (minimum guaranteed size).
    // This is safer than relying solely on a 'wait loop' outside this function.
    if(actualWidth == 0) {
        actualWidth = capabilities->minImageExtent.width;
    }
    if(actualHeight == 0) {
        actualHeight = capabilities->minImageExtent.height;
    }

    VkExtent2D actualExtent = {
        .width = actualWidth,
        .height = actualHeight
    };

    // Clamp the size between minImageExtent and maxImageExtent as per Vulkan spec.
    actualExtent.width = CLAMP(actualExtent.width,
                               capabilities->minImageExtent.width,
                               capabilities->maxImageExtent.width);
    actualExtent.height = CLAMP(actualExtent.height,
                                capabilities->minImageExtent.height,
                                capabilities->maxImageExtent.height);

    return actualExtent;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    if(device == VK_NULL_HANDLE) return false;

    // Check queue families
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    if(!is_indices_complete(&indices)) {
        return false;
    }

    // Check required extensions
    if(!checkDeviceExtensionSupport(device)) {
        return false;
    }

    // Check swapchain support
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
    bool swapChainAdequate =
        swapChainSupport.formats_count > 0 &&
        swapChainSupport.presentModes_count > 0;

    free(swapChainSupport.formats);
    free(swapChainSupport.presentModes);

    if(!swapChainAdequate) {
        return false;
    }

    return true;
}


uint8_t *readFile(const char *filename, size_t *pCodeSize) {

    // Safety check 1: Validate required input pointers.
    if(filename == NULL || pCodeSize == NULL) {
        // SDL_Log("Invalid input: filename or pCodeSize pointer is NULL.");
        return NULL;
    }

    // Initialize output size to zero on failure path.
    *pCodeSize = 0;

    // 1. Open file using fopen_s
    FILE *file = NULL; // Must be initialized to NULL for fopen_s
    errno_t err = fopen_s(&file, filename, "rb");

    // Check the error code returned by fopen_s.
    if(err != 0 || file == NULL) {
        // SDL_Log("Failed to open shader file '%s'. errno_t: %d", filename, err);
        return NULL;
    }

    // 2. Determine file size
    if(fseek(file, 0, SEEK_END) != 0) {
        // SDL_Log("Failed to seek to end of file: %s", filename);
        fclose(file);
        return NULL;
    }

    long fileSizeLong = ftell(file);
    if(fileSizeLong == -1L) {
        // SDL_Log("Failed to determine file size (ftell error): %s", filename);
        fclose(file);
        return NULL;
    }

    // Safety check 3: Ensure file size fits in size_t and is non-zero
    if(fileSizeLong <= 0) { // Check for non-positive size
        // SDL_Log("File size is invalid (negative or zero): %s", filename);
        fclose(file);
        return NULL;
    }

    size_t fileSize = (size_t)fileSizeLong;
    *pCodeSize = fileSize;

    // Reset file pointer to beginning
    if(fseek(file, 0, SEEK_SET) != 0) {
        // SDL_Log("Failed to seek to beginning of file: %s", filename);
        fclose(file);
        return NULL;
    }

    // 3. Allocate buffer
    uint8_t *buffer = (uint8_t *)malloc(fileSize);
    if(!buffer) {
        // SDL_Log("Failed to allocate memory for shader code: %s", filename);
        fclose(file);
        return NULL;
    }

    // 4. Read file into buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if(bytesRead != fileSize) {
        // SDL_Log("Failed to read all bytes from shader file '%s'. Read %zu of %zu bytes.", 
        //         filename, bytesRead, fileSize);

        // Cleanup on failure
        free(buffer);
        fclose(file);
        *pCodeSize = 0; // Reset size on failure
        return NULL;
    }

    // 5. Success
    fclose(file);
    return buffer;
}

VkShaderModule createShaderModule(VkDevice device, const uint8_t *code, size_t codeSize) {

    // Define a safe failure value
    VkShaderModule shaderModule = VK_NULL_HANDLE;

    // Safety check 1: Validate required input handles and pointers.
    if(device == VK_NULL_HANDLE) {
        // SDL_Log("Invalid input: VkDevice is VK_NULL_HANDLE.");
        return shaderModule;
    }
    if(code == NULL) {
        // SDL_Log("Invalid input: Shader code pointer is NULL.");
        return shaderModule;
    }
    if(codeSize == 0) {
        // SDL_Log("Invalid input: Shader code size is 0.");
        return shaderModule;
    }

    // Safety check 2: Validate that codeSize is a multiple of 4.
    // SPIR-V code must be an array of 32-bit words.
    if(codeSize % 4 != 0) {
        // SDL_Log("Invalid input: Shader code size (%zu bytes) is not a multiple of 4!", codeSize);
        return shaderModule;
    }

    // Initialize create info structure
    VkShaderModuleCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    // The pointer cast is required by the Vulkan API for SPIR-V, 
    // but the 'multiple of 4' check makes this cast safe regarding memory alignment/size.
    createInfo.pCode = (const uint32_t *)code;

    // 3. Create the Shader Module
    VkResult result = vkCreateShaderModule(device, &createInfo, NULL, &shaderModule);

    // Safety check 3: Check Vulkan return code
    if(result != VK_SUCCESS) {
        // SDL_Log("Failed to create shader module! VkResult: %d", result);
        // shaderModule is already VK_NULL_HANDLE in this failure case.
    }

    // Return VK_NULL_HANDLE on failure, or the valid module on success.
    return shaderModule;
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    if(physicalDevice == VK_NULL_HANDLE) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "findMemoryType: Invalid context or physicalDevice!");
        return UINT32_MAX;
    }

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if((typeFilter & (1 << i)) &&
           (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "findMemoryType: Failed to find suitable memory type!");
    return UINT32_MAX; // Caller must handle this as an error
}