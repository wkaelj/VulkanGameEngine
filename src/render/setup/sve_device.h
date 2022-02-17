#ifndef INIT_VK
#define INIT_VK
// set of functions to create and configure a vulkan instance

#include <sve_header.h>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utilities/fileutils.h>



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

// struct to store vulkan swapchain data
typedef struct {
    VkSwapchainKHR swapchain; // window swapchain
    VkImage *images; // swapchain images, gets rallocated in createSwapchain()
    uint32_t imageCount; // variable to store number of swapchain images (both view and normal)
    VkImageView *imageViews; // swapchain image views
    VkFormat imageFormat;
    VkExtent2D imageExtent;
} SveSwapchainData;



//
// Public Functions
//

// function to initialize a vulkan instance
int sveCreateDevice (SveDeviceCreateInfo *initInfo);

// function to update the glfw window created by initvk_initVulkan
// Returns "EXIT_FAILURE" if window should close
int sveUpdateWindow (void);

// return dimensions of swapchain window
int sveGetWindowSize (uint32_t *pWidth, uint32_t *pHeight);

// set a pointer to the vulkan device
int sveGetDevice (VkDevice *vulkanDevice);

// set a pointer to the swapchain data
int sveGetSwapchain (SveSwapchainData *pSwapchainData);

// terminate vulkan instanse and clean it up
int sveDestroyDevice (void);

#endif