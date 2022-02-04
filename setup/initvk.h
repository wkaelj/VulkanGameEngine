#ifndef INIT_VK
#define INIT_VK
// set of functions to create and configure a vulkan instance

#include <stdbool.h>
#include <vulkan/vulkan.h>

typedef struct _VkInitInfo {
    char *windowName;
    bool windowResizable;
    bool windowFullscreen;
    uint16_t windowWidth;
    uint16_t windowHeight;
    bool activateValidation;
    bool preferIntegratedGPU;
} VkInitInfo;

// function to initialize a vulkan instance
int initvk_initVulkan (VkInitInfo *initInfo);

// function to update the glfw window created by initvk_initVulkan
// Returns "EXIT_FAILURE" if window should close
int initvk_updateWindow (void);

// terminate vulkan instanse and clean it up
int initvk_cleanVulkan (void);

#endif