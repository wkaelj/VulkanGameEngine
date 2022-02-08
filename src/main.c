// temp main function
#include <stdio.h>
#include <stdlib.h>
#include "render/setup/initvk.h"
#include "debug/debug.h"
#include "utilities/fileutils.h"

int mainLoopTest (void);

int main () {
	//mainLoopTest ();
	// debug_log ("Hello World!%i%s", 5, "Pizza");

	char **out;
	uint32_t length;
	readFileCharArray ("/home/kael/Code/VulkanGameEngine/src/utilities/string.txt", out, &length);

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