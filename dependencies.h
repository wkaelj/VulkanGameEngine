#ifndef DEPENDENCIES_H_ // header guard
#define DEPENDENCIES_H_

//Cmake stuffs
#define SimpleVulkanEngine_VERSION_MAJOR @Tutorial_VERSION_MAJOR@
#define SimpleVulkanEngine_VERSION_MINOR @Tutorial_VERSION_MINOR@

// include glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// include vulkan
#include <vulkan/vulkan.h>

// standard libraries
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>

// some global things to include
#include "debug/debug.h"

// random definitions that are sometimes useful
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))
#define NEW(x) ((x*) malloc (sizeof(x))) // initialize pointers

#endif // header guard
