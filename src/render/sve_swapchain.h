// function to create vulkan swapchains
// written Feb 20 2022

#include <sve_header.h>
#include <utilities/utilities.h>
#include "sve_device.h"



//
// Structure Definitions
//


//
// Public functions
//

// function to initialize swapchain
int sveInitSwapchain (void);

// recreate swapchain when window is resized
int sveResizeSwapchain (void);

// cleanup swapchain
int sveDestroySwapchain (void);