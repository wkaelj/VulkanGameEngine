#ifndef INIT_VK
#define INIT_VK
// set of functions to create and configure a vulkan instance

#include <stdbool.h>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>
#include "../../debug/debug.h"
#include "../../utilities/fileutils.h"
#include <assert.h>

// structure to define information about shader config file
typedef struct {
    char *configFilePath;
    char argBreakChar;
    size_t paramCount;
} SveShaderModuleLoaderInfo;

// info to intitialize a vulkan instance
typedef struct {
    char *windowName;
    bool windowResizable;
    bool windowFullscreen;
    uint16_t windowWidth;
    uint16_t windowHeight;
    bool activateValidation;
    bool preferIntegratedGPU;
    SveShaderModuleLoaderInfo *shaderLoaderInfo;
} SveVkInitInfo;

// struct to store vulkan swapchain data
typedef struct {
    VkSwapchainKHR swapchain; // window swapchain
    VkImage images; // swapchain images, gets rallocated in createSwapchain()
    uint32_t imageCount; // variable to store number of swapchain images (both view and normal)
    VkImageView *imageViews; // swapchain image views
    VkFormat imageFormat;
    VkExtent2D imageExtent;
} SveSwapchainData;

// function to initialize a vulkan instance
int sveInitVulkan (SveVkInitInfo *initInfo);

// function to update the glfw window created by initvk_initVulkan
// Returns "EXIT_FAILURE" if window should close
int sveUpdateWindow (void);

// terminate vulkan instanse and clean it up
int sveCleanVulkan (void);

#endif