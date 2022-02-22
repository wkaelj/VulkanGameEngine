// swapchain implementation
// Kael Johnston Feb 20 2022

#include "sve_swapchain.h"

//
// Structures
//

//
// Variables
//
VkSwapchainKHR swapchain = VK_NULL_HANDLE;

//
// Private Functions
//

// function to check for swapchain suppor4t

// function to create image views
int createImageViews (void);

VkSwapchainKHR createSwapchain (SwapChainSupportDetails supportDetails, VkSwapchainKHR oldSwapchain);

//
// Public Function Implementation
//

int sveInitSwapchain (void) {

    swapchain = createSwapchain (*sveGetSwapchainSupport (NULL), swapchain);
    return SUCCESS;
}

int sveResizeSwapchain (void) {

    swapchain = createSwapchain (*sveGetSwapchainSupport (NULL), swapchain);

    return SUCCESS;
}

int sveDestroySwapchain (void) {

    if (sveGetDevice () == VK_NULL_HANDLE) {
        LOG_INFO("Cannot destroy swapchain, no device");
        return FAILURE;
    }

    if (swapchain == VK_NULL_HANDLE) return SUCCESS;
    vkDestroySwapchainKHR (sveGetDevice (), swapchain, NULL);
    swapchain = VK_NULL_HANDLE;

    return SUCCESS;
}

//
// Private Function Helpers
//

// swapchain
SwapChainSupportDetails querySwapChainSupport (VkPhysicalDevice questionedDevice);

// choose desired or best swap chain mode
VkSurfaceFormatKHR chooseSwapSurfaceFormat (uint32_t formatCount, const VkSurfaceFormatKHR *availableFormats);


// choose best or desired surface format
VkPresentModeKHR chooseSwapPresentMode (uint32_t presentModesCount, const VkPresentModeKHR *availablePresentModes);
// find screen resolution for swap
VkExtent2D chooseSwapExtent (VkSurfaceCapabilitiesKHR *capabilities);

//
// Private Function Implementation
//

VkSwapchainKHR createSwapchain (SwapChainSupportDetails supportDetails, VkSwapchainKHR oldSwapchain) {
    
    VkSwapchainKHR out = VK_NULL_HANDLE;

    if (sveGetDevice () == VK_NULL_HANDLE) {
        LOG_INFO("Cannot create swapchain without first creating a device Creating device automaticaly.");
        SveDeviceCreateInfo deviceInfo = {};
        deviceInfo.activateValidation = true;
        deviceInfo.preferIntegratedGPU = false;
        deviceInfo.windowFullscreen = false;
        deviceInfo.windowHeight = 600;
        deviceInfo.windowWidth = 800;
        deviceInfo.windowName = "Failed to correctly initailize engine";
        deviceInfo.windowResizable = true;
        sveCreateDevice (&deviceInfo);
    }
    if (sveGetDevice () == VK_NULL_HANDLE) {
        LOG_FATAL ("Failed to intialize device automaticly.");
        return VK_NULL_HANDLE;
    }
    #define swapchainDetails sveGetSwapchainSupport (NULL)

    // choose format
    assert (swapchainDetails != NULL);
    VkPresentModeKHR presentMode = chooseSwapPresentMode (swapchainDetails->presentModeCount, swapchainDetails->presentModes);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat (swapchainDetails->formatCount, swapchainDetails->formats);
    VkExtent2D swapExtent = chooseSwapExtent (&swapchainDetails->capabilties);

    // image count
    uint32_t swapImageCount = swapchainDetails->capabilties.minImageCount + 1;
    if (swapchainDetails->capabilties.maxImageCount > 0 && swapImageCount > swapchainDetails->capabilties.maxImageCount) {
        swapImageCount = swapchainDetails->capabilties.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapCreateInfo = {};
    swapCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapCreateInfo.minImageCount = swapImageCount;
    swapCreateInfo.imageFormat = surfaceFormat.format;
    swapCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapCreateInfo.imageExtent = swapExtent;
    swapCreateInfo.imageArrayLayers = 1;
    swapCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    swapCreateInfo.surface = sveGetWindowSurface ();

    // queue families
    QueueFamilyIndices indices = *sveGetQueueFamilies (NULL); 
    uint32_t queueFamilies[] = QUEUE_NAMES(indices);

    if (indices.graphicsFamily != indices.presentFamily) {
        swapCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapCreateInfo.queueFamilyIndexCount = QUEUE_COUNT;
        swapCreateInfo.pQueueFamilyIndices = queueFamilies;
    } else {
        swapCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapCreateInfo.queueFamilyIndexCount = 0; // Optional
        swapCreateInfo.pQueueFamilyIndices = NULL; // Optional
    }

    swapCreateInfo.preTransform = swapchainDetails->capabilties.currentTransform;
    // alpa is not for rendering, but for window system.
    swapCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapCreateInfo.presentMode = presentMode;
    swapCreateInfo.clipped = VK_TRUE;

    swapCreateInfo.oldSwapchain = oldSwapchain; // for swapchain recreation
    
    if (vkCreateSwapchainKHR (sveGetDevice (), &swapCreateInfo, NULL, &out) != VK_SUCCESS) {
        
        LOG_ERROR("Failed to create swapchain");
        return VK_NULL_HANDLE;
    } else LOG_DEBUG ("Created Swapchain");

    #undef swapchainDetails
    return out;
    }

// swapchain
VkSurfaceFormatKHR chooseSwapSurfaceFormat (uint32_t formatCount, const VkSurfaceFormatKHR *availableFormats) {

    for (uint32_t i = 0; i < formatCount; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode (uint32_t presentModeCount, const VkPresentModeKHR *availablePresentModes) {

    // return power saving present mode if using integrated GPU
    // FIXME if (preferIntegratedGPU) { return VK_PRESENT_MODE_FIFO_KHR; }

    // check if prefered render mode is available
    for (uint32_t i = 0; i < presentModeCount; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent (VkSurfaceCapabilitiesKHR *capabilities) {

    // if (capabilities->currentExtent.width != UINT32_MAX) {
    //     return capabilities->currentExtent;
    // }

    int32_t width, height; // width and hight of swapchain buffer

    sveGetWindowSize (&width, &height);

    // clamp dimensinos to fit swapchain capabilities
    width = clampValue (capabilities->minImageExtent.width, capabilities->maxImageExtent.width, width);
    height = clampValue (capabilities->minImageExtent.height, capabilities->maxImageExtent.height, height);

    VkExtent2D actualExtent = {
        (uint32_t) width,
        (uint32_t) height
    };

    return actualExtent;
}

//
// Helper Function Implementation
//

