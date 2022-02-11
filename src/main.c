// temp main function
#include <stdio.h>
#include <stdlib.h>
#include "render/setup/initvk.h"
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
	SveVkInitInfo initInfo = {};
	initInfo.windowName = "Hello World";
	initInfo.windowWidth = 800;
	initInfo.windowHeight = 600;
	initInfo.windowFullscreen = false;
	initInfo.windowResizable = false;
	initInfo.activateValidation = true;
	initInfo.shaderLoaderInfo = &loaderInfo;

	// initialize vulkan
	sveInitVulkan (&initInfo);
	
	// very important loop
	while (sveUpdateWindow () == EXIT_SUCCESS) {

	}

	// fix my garbage
	sveCleanVulkan ();
	return EXIT_SUCCESS;
}