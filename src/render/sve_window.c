#include "sve_window.h"

//
// Structures
//

//
// Variables
//
GLFWwindow *window = NULL;
VkSurfaceKHR surface = NULL;

//
// Private Functions
//

//
// Public Function Implementation
//

int sveCreateWindow (uint32_t w, uint32_t h, char *name, bool resizable) {

    if (glfwInit () != GLFW_TRUE) return FAILURE;

    glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API); // disable opengl api

    // resizable
    if (resizable) glfwWindowHint (GLFW_RESIZABLE, GLFW_TRUE);
    else glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow (w, h, name, NULL, NULL);

    // idk how to tell if this fails lol
    return SUCCESS;
}

int sveDestroyWindow (void) {
    glfwDestroyWindow (window);

    window = NULL;

    glfwTerminate ();

    return SUCCESS;
}

int sveGetWindowSize (int32_t *w, int32_t *h) {

    if (window == NULL) return FAILURE;

    glfwGetFramebufferSize (window, w, h);

    return SUCCESS;
}

int sveUpdateWindow (void) {
    glfwPollEvents ();
    if (glfwWindowShouldClose (window)) return FAILURE;
    else return SUCCESS;
}

int sveUpdateWindowTitle (const char *title) {

    if (window == NULL) return FAILURE;

    glfwSetWindowTitle (window, title);

    return SUCCESS;
}

int sveCreateWindowSurface (VkInstance instance) {

    if (glfwCreateWindowSurface (instance, window, NULL, &surface) != VK_SUCCESS) return FAILURE;

    return SUCCESS;
}

int sveDestroyWindowSurface (VkInstance instance) {

    if (surface != NULL) vkDestroySurfaceKHR (instance, surface, NULL);
    else return FAILURE;// all the keywords

    return SUCCESS;
}

int sveGetRequiredWindowExtensions (char ***ppExtensionsString, uint32_t *pExtensionsCount) {

    uint32_t extensionCount = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions (&extensionCount);

    *ppExtensionsString = (char **) extensions;
    *pExtensionsCount = extensionCount;
    return SUCCESS;
}
VkSurfaceKHR sveGetWindowSurface (void) {
    return surface;
}

//
// Private Function Implementation
//