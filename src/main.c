// temp main function
#include <stdio.h>
#include <stdlib.h>

int mainLoopTest (void);

int main () {

	// mainLoopTest ();
	debug_log ("Hello World!%li", 5);
	return EXIT_SUCCESS;
}


int mainLoopTest (void) {

	// populate inint info struct
	VkInitInfo initInfo = {};
	initInfo.windowName = "Hello World";
	initInfo.windowWidth = 800;
	initInfo.windowHeight = 600;
	initInfo.windowFullscreen = false;
	initInfo.windowResizable = false;
	initInfo.activateValidation = true;

	// initialize vulkan
	initvk_initVulkan (&initInfo);
	
	// very important loop
	while (initvk_updateWindow () == EXIT_SUCCESS) {

	}

	// fix my garbage
	initvk_cleanVulkan ();
	return EXIT_SUCCESS;
}