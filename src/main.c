// temp main function
#include <stdio.h>
#include <stdlib.h>
#include "render/setup/initvk.h"
#include "debug/debug.h"
#include "utilities/fileutils.h"

int mainLoopTest (void);

int main () {

	uint32_t length;
	readFileStringArray ("/home/kael/Code/VulkanGameEngine/src/utilities/string.txt", NULL, &length);
	char *out[length];
	readFileStringArray ("/home/kael/Code/VulkanGameEngine/src/utilities/string.txt", out, &length);
	debug_log ("Array length = %i", length);

	for (size_t i = 0; i < length; i++) debug_log ("String: %s", out[i]);

	mainLoopTest ();

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