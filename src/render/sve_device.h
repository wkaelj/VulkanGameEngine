#ifndef INIT_VK
#define INIT_VK
// set of functions to create and configure a vulkan instance

#include <sve_header.h>
#define GLFW_INCLUDE_VULKAN

#define QUEUE_COUNT 2
#define QUEUE_NAMES(varname) {varname.graphicsFamily, varname.presentFamily}


// info to intitialize a vulkan instance
typedef struct {
    char *windowName;
    bool windowResizable;
    bool windowFullscreen;
    uint16_t windowWidth;
    uint16_t windowHeight;
    bool activateValidation;
    bool preferIntegratedGPU;
} SveDeviceCreateInfo;


// struct to store swapchain creation in formation
typedef struct {
    VkSurfaceCapabilitiesKHR capabilties;
    VkSurfaceFormatKHR *formats; // maybe need to fix pointers later
    uint32_t formatCount;
    VkPresentModeKHR *presentModes;
    uint32_t presentModeCount;
} SwapChainSupportDetails;

// struct to store queue creation data
typedef struct {

    // grapics family
    uint32_t graphicsFamily;
    bool foundGraphicsFamily;

    // presentation family
    uint32_t presentFamily;
    bool foundPresentFamily;
    
} QueueFamilyIndices;

//
// Public Functions
//

// function to initialize a vulkan instance
int sveCreateDevice (SveDeviceCreateInfo *initInfo);

// function to update the glfw window created by initvk_initVulkan
// Returns "EXIT_FAILURE" if window should close
int sveUpdateWindow (void);

// return dimensions of swapchain window
int sveGetWindowSize (int32_t *pWidth, int32_t *pHeight);

// get the vulkan device
VkDevice sveGetDevice (void);

// returns graphics queue families
QueueFamilyIndices *sveGetQueueFamilies (VkPhysicalDevice optionalDevice);

// get swapchain support details. Input NULL for current device in use.
// returns null on failure
SwapChainSupportDetails *sveGetSwapchainSupport (VkPhysicalDevice optionalDevice);

// terminate vulkan instanse and clean it up
int sveDestroyDevice (void);

#endif