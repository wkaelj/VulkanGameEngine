// window creation and managment
// Kael Johnston Feb 20 2022

#include <sve_header.h>
#include<GLFW/glfw3.h>

// create a new window
int sveCreateWindow (uint32_t w, uint32_t h, char *name, bool resizable, bool fullscreen);

// get window dimensions
int sveGetWindowSize (int32_t *w, int32_t *h);

// function to create a window surface
int sveCreateWindowSurface (VkInstance instance);

// destroy window
int sveDestroyWindow (void);

// destroy window surface
int sveDestroyWindowSurface (VkInstance instance);

int sveUpdateWindow (void);

// enable/disable window fullscreen
int sveToggleWindowFullscreen (bool fullscreen);

int sveGetRequiredWindowExtensions (char ***ppExtensionsString, uint32_t *pExtensionsCount);
// get window surface
VkSurfaceKHR sveGetWindowSurface (void);