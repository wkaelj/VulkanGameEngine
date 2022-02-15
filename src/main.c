// temp main function
#include <stdio.h>
#include <stdlib.h>
#include "render/setup/sve_device.h"
#include "render/setup/sve_pipeline.h"
#include "debug/debug.h"
#include "utilities/fileutils.h"

int mainLoopTest (void);

int main () {

	mainLoopTest ();

	return EXIT_SUCCESS;
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
	deviceInfo.windowFullscreen = false;
	deviceInfo.windowResizable = false;
	deviceInfo.activateValidation = true;

	SvePipelineCreateInfo piplineInfo = {};
	piplineInfo.shaderLoaderInfo = &loaderInfo;
	// initialize vulkan
	if (sveCreateDevice (&deviceInfo) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (sveInitGraphicsPipeline (&piplineInfo) != EXIT_SUCCESS) return EXIT_FAILURE;
	
	// very important loop
	while (sveUpdateWindow () == EXIT_SUCCESS) {

	}

	// fix my garbage
	sveCleanGraphicsPipeline ();
	sveDestroyDevice ();
	return EXIT_SUCCESS;
}