// temp main function
#include "render/sve_device.h"
#include "render/sve_pipeline.h"
#include "render/sve_swapchain.h"
#include "render/sve_window.h"

#include <utilities/datastructures/linked_list.h>

int mainLoopTest (void);

int main () {

    mainLoopTest ();

    return SUCCESS;
}

// temp mainloop functions
int mainLoopTest (void) {

        // tell vulkan where to load shaders from
    SveShaderModuleLoaderInfo loaderInfo = {};
    loaderInfo.configFilePath = "Config/shaders_list.txt";
    loaderInfo.paramCount = 3;
    loaderInfo.argBreakChar = ';';

    // populate inint info struct
    SveDeviceCreateInfo deviceInfo = {};
    deviceInfo.windowName = "Hello World";
    deviceInfo.windowWidth = 800;
    deviceInfo.windowHeight = 600;
    deviceInfo.windowFullscreen = true;
    deviceInfo.windowResizable = false;
    deviceInfo.activateValidation = true;

    SvePipelineCreateInfo piplineInfo = {};
    piplineInfo.shaderLoaderInfo = &loaderInfo;
    // initialize vulkan
    if (sveCreateDevice (&deviceInfo) != EXIT_SUCCESS) return EXIT_FAILURE;
    if (sveInitSwapchain () != SUCCESS) return FAILURE;
    
    // very important loop
    while (sveUpdateWindow () == SUCCESS) {

    }

    sveDestroySwapchain ();
    sveDestroyDevice ();
    return EXIT_SUCCESS;
}
