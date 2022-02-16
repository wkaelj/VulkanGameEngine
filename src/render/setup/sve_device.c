// initvk implementation file

#include "sve_device.h"
#include <string.h> // FIXME tmp

//
// Magic Number Definitions
//

#define QUEUE_COUNT 2
#define QUEUE_NAMES (indices.graphicsFamily, indices.presentFamily)

//
// Struct definitions & implementations
//

// structure to manage rendering device candidates
typedef struct {
    VkPhysicalDevice physicalDevice;
    uint16_t deviceScore;
} SuitableDevice;

// struct to store queue creation data
typedef struct {

    // grapics family
    uint32_t graphicsFamily;
    bool foundGraphicsFamily;

    // presentation family
    uint32_t presentFamily;
    bool foundPresentFamily;
    
} QueueFamilyIndices;

// struct to store swapchain creation in formation
typedef struct {
    VkSurfaceCapabilitiesKHR capabilties;
    VkSurfaceFormatKHR *formats; // maybe need to fix pointers later
    uint32_t formatCount;
    VkPresentModeKHR *presentModes;
    uint32_t presentModeCount;
} SwapChainSupportDetails;

//
// Variable Declerations
//

VkInstance instance; // vulkan instance
VkPhysicalDevice physicalDevice; // vulkan rendering device
VkDevice device; // logical vulkan device
VkQueue graphicsQueue; // vulkan graphics queue
VkQueue presentQueue; // vulkan presentation queue
GLFWwindow *window; // glfw window
VkSurfaceKHR windowSurface; // vulkan window surface
VkDebugUtilsMessengerEXT debugMessenger; // vulkan debug messenger
VkRenderPass renderPass; // vulkan render passes
VkPipelineLayout pipelineLayout; // vulkan pipeline layout
bool preferIntegratedGPU; // wether to prefer a integrated GPU over a discrete GPU
SveSwapchainData swapchain = {}; // store swapchain data


// required validation layers (wanna put in config file)
const char *requiredLayerNames[] = {"VK_LAYER_KHRONOS_validation"}; // requred validation layers
const char *requiredDeviceExtensionNames[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; // required GPU extensions
bool enableValidationLayers = true; // if the program should enable extra debug functionality (yes by default)

//
// Definition definitions
//

// inline functions to clamp a value
inline int64_t clampValue(uint32_t min, uint32_t max, uint32_t val) {
    if (val < max && val > min) {
        return val;
    } else if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    }

    return 0;
}

#define arrayLength(x) (sizeof (x) / sizeof (x[0]))

//
// Private Function Declerations
//

// create a glfw window
int createWindow (SveDeviceCreateInfo *info);
// create a vulkan instance and store it in instance
int createInstance (void);
// create a glfw window surface, and store it in windowSurface or smthn
int createWindowSurface (void);
// select a physical device and store it in physicalDevice
int selectPhysicalDevice (void);
// test if questionedDevice is suitable for rendering, 0 means no, higher score is better
uint16_t isDeviceSuitable (VkPhysicalDevice questionedDevice);
// check if questionedDevice supports all of the etensions
int checkDeviceExtensionSupport (VkPhysicalDevice questionedDevice);
// find all questionedDevice's queues
QueueFamilyIndices findDeviceQueue (VkPhysicalDevice questionedDevice);
// create VkDevice and all the VkQueues
int createLogicalDevice (void);
// function to create a swapchain
int createSwapchain (void);
// function to create the image views
int createImageViews (void);
// check if questionedDevice supports the swapchain
SwapChainSupportDetails querySwapChainSupport (VkPhysicalDevice questionedDevice);
// choose desired or best swap chain mode
VkSurfaceFormatKHR chooseSwapSurfaceFormat (uint32_t formatCount, const VkSurfaceFormatKHR *availableFormats);
// choose best or desired surface format
VkPresentModeKHR chooseSwapPresentMode (uint32_t presentModesCount, const VkPresentModeKHR *availablePresentModes);
// find screen resolution for swap
VkExtent2D chooseSwapExtent (VkSurfaceCapabilitiesKHR *capabilities);
// function loaders (broken)
VkResult CreateDebugUtilsMessengerEXT (VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger);

void DestroyDebugUtilsMessengerEXT ( VkInstance instance,
    VkDebugUtilsMessengerEXT *pDebugMessenger,
    const VkAllocationCallbacks *pAllocator);

// 
// Public Functions
// 
int sveCreateDevice (SveDeviceCreateInfo *initInfo) {

    // manage struct inputs
    enableValidationLayers = initInfo->activateValidation;
    preferIntegratedGPU = initInfo->preferIntegratedGPU;

    // run init functions

    // create a glfw window
    if (createWindow (initInfo) != EXIT_SUCCESS) {
        debug_log ("Failed to create window.");
        return EXIT_FAILURE;
    }
    if (createInstance () != EXIT_SUCCESS) {
        debug_log ("Failed to create vulkan instance.");
        return EXIT_FAILURE;
    }
    // create window surface
    if (createWindowSurface () != EXIT_SUCCESS) {
        debug_log ("Failed to create GLFW window surface");
    }
    // select gpu
    if (selectPhysicalDevice () != EXIT_SUCCESS) {
        debug_log ("Failed to select a physical device.");
        return EXIT_FAILURE;
    }
    // create a device
    if (createLogicalDevice () != EXIT_SUCCESS) {
        debug_log ("Failed to create logical device.");
        return EXIT_FAILURE;
    }
    // create swapchain
    if (createSwapchain () != EXIT_SUCCESS) {
        debug_log ("Failed to create swapchain.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int sveUpdateWindow (void) {
    glfwPollEvents ();

    if (glfwWindowShouldClose (window)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int sveDestroyDevice (void) {

    vkDestroyRenderPass (device, renderPass, NULL);

    // FIXME segfault
    // destroy swapchain image views
    for (int i = 0; i < swapchain.imageCount; i++) {
        vkDestroyImageView(device, swapchain.imageViews[i], NULL);
    }

    vkDestroySwapchainKHR (device, swapchain.swapchain, NULL);
    vkDestroyDevice (device, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT  (instance, &debugMessenger, NULL);
    }

    vkDestroySurfaceKHR (instance, windowSurface, NULL);

    vkDestroyInstance (instance, NULL);

    glfwDestroyWindow (window);

    glfwTerminate ();

    return EXIT_SUCCESS;
}

int sveGetWindowSize (uint32_t *pWidth, uint32_t *pHeight) {
    if (pWidth == NULL) pWidth = malloc (sizeof (uint32_t));
    if (pHeight == NULL) pHeight = malloc (sizeof (uint32_t));

    *pWidth = swapchain.imageExtent.width;
    *pHeight = swapchain.imageExtent.height;

    return EXIT_SUCCESS;
}

int sveGetDevice (VkDevice *vulkanDevice) {
    assert (&device != NULL);
    *vulkanDevice = device;
    assert (vulkanDevice != NULL);
    assert (*vulkanDevice == device);

    return EXIT_SUCCESS;
}

int sveGetSwapchain (SveSwapchainData *pSwapchainData) {

    if (swapchain.swapchain == NULL) return EXIT_FAILURE;
    *pSwapchainData = swapchain;

    return EXIT_SUCCESS;
}

//
// Private function implementation
// 

// function to create a glfw window
int createWindow (SveDeviceCreateInfo *info) {

    glfwInit ();
    
    glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
    if (info->windowResizable)
        glfwWindowHint (GLFW_RESIZABLE, GLFW_TRUE);
    else
        glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow (info->windowWidth, info->windowHeight, info->windowName, NULL, NULL);

    // i dont think this i can catch this if it fails lol
    return EXIT_SUCCESS;
}

// function to create window surface
int createWindowSurface (void) {

    if (glfwCreateWindowSurface (instance, window, NULL, &windowSurface) != VK_SUCCESS) {
        return EXIT_FAILURE;
    }

    return  EXIT_SUCCESS;
}

int checkValidationLayerSupport () {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (uint32_t i = 0; i < arrayLength(requiredLayerNames); i++)
    {
        bool found = false;

        //iterate through available extensions
        for (uint32_t x = 0; x < layerCount; x++) {
            if (strcmp (requiredLayerNames[i], availableLayers[x].layerName) == EXIT_SUCCESS) {
                debug_log ("Found layer: %s", availableLayers[x].layerName);
                found = true;
                break;
            }
        }

        if (found == false) {
            debug_log("Unavailable valiation layer: %s", requiredLayerNames[i]);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    debug_log("Validation Layer: %s", pCallbackData->pMessage);

    return VK_FALSE;
}
// function to initialize a vulkan instance
int createInstance (void) {
    
    // check if validation support is available
    if (enableValidationLayers && checkValidationLayerSupport() != EXIT_SUCCESS) {
        debug_log("validation layers requested, but not available!");
        enableValidationLayers = false;
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // get glfw extensinos
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    int32_t extensionCount = glfwExtensionCount + 1;
    const char* extensions[extensionCount];
    for (int i = 0; i < glfwExtensionCount; i++)
    {
        extensions[i] = glfwExtensions[i];
    }
    extensions[extensionCount-1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;

    // validation layers
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = arrayLength (requiredLayerNames);
        createInfo.ppEnabledLayerNames = requiredLayerNames;
        
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        
        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }

    if (vkCreateInstance (&createInfo, NULL, &instance) != VK_SUCCESS) {
        debug_log ("Failed to create vulkan instance");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// select a physical rendering device
int selectPhysicalDevice (void) {

    // set device to null so i can mess with it
    physicalDevice = VK_NULL_HANDLE;

    // check available rendering devices
    uint32_t availableDeviceCount = 0;
    vkEnumeratePhysicalDevices (instance, &availableDeviceCount, NULL);
    //throw error if there are no devices
    if (availableDeviceCount == 0) {
        debug_log ("No GPU detected.");
        return EXIT_FAILURE;
    }

    VkPhysicalDevice availableDevices[availableDeviceCount];
    vkEnumeratePhysicalDevices (instance, &availableDeviceCount, availableDevices);

    uint32_t suitableDeviceCount = 0;
    SuitableDevice suitableDevices[availableDeviceCount];

    uint16_t tmpDeviceOutput = 0; // used to store function output in for loop
    for (uint32_t i = 0; i < availableDeviceCount; i++) {
        tmpDeviceOutput = isDeviceSuitable (availableDevices[i]);
        if (tmpDeviceOutput > 0) {
            suitableDevices[suitableDeviceCount].physicalDevice = availableDevices[i];
            suitableDevices[suitableDeviceCount].deviceScore = tmpDeviceOutput;
            suitableDeviceCount++;
        }
    }

    // check if there are no suitable GPU
    if (suitableDeviceCount == 0) {
        debug_log ("No suitable GPU detected.");
        return EXIT_FAILURE;
    }

    // select highest ranking device
    SuitableDevice outputDevice;
    outputDevice.deviceScore = 0;
    outputDevice.physicalDevice = VK_NULL_HANDLE;
    for (uint16_t i = 0; i < suitableDeviceCount; i++) {

        if (suitableDevices[i].deviceScore > outputDevice.deviceScore) {
            outputDevice.deviceScore = suitableDevices[i].deviceScore;
            outputDevice.physicalDevice = suitableDevices[i].physicalDevice;
        }
    }

    // set device to highest ranking device
    physicalDevice = outputDevice.physicalDevice;

    return EXIT_SUCCESS;
}

// function to create a logical device
int createLogicalDevice (void) {

    QueueFamilyIndices indices = findDeviceQueue (physicalDevice);

    // create queues
    float queuePriority = 1.0f; // float pointer for later
    uint32_t queueCount = QUEUE_COUNT; // queue count
    uint32_t queueFamilies[QUEUE_COUNT] = {QUEUE_NAMES}; // queue indices
    VkDeviceQueueCreateInfo queueCreateInfos[queueCount]; // queue create infos

    #warning fixme same queue
    for (uint32_t i = 0; i < queueCount; i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {}; // populate later

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;

    createInfo.pEnabledFeatures = &deviceFeatures;

    // enable device extensions
    createInfo.enabledExtensionCount = arrayLength (requiredDeviceExtensionNames);
    createInfo.ppEnabledExtensionNames = requiredDeviceExtensionNames;

    // pass used enabled validation layers
    createInfo.enabledLayerCount = arrayLength(requiredLayerNames);
    createInfo.ppEnabledLayerNames = requiredLayerNames;

    if (vkCreateDevice (physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS) {
        // debug function
        return EXIT_FAILURE;
    }
    
    vkGetDeviceQueue (device, indices.graphicsFamily, 0, &graphicsQueue); // create graphics queue
    vkGetDeviceQueue (device, indices.presentFamily, 0, &presentQueue); // create present queue

    return EXIT_SUCCESS;
}

// create swapchain
int createSwapchain (void) {

    SwapChainSupportDetails swapDetails = querySwapChainSupport(physicalDevice);

    // choose surface format, present and extent
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat (swapDetails.formatCount, swapDetails.formats);
    VkPresentModeKHR surfacePresentMode = chooseSwapPresentMode (swapDetails.presentModeCount, swapDetails.presentModes);
    VkExtent2D surfaceExtent = chooseSwapExtent (&swapDetails.capabilties);

    // pick image count
    uint32_t imageCount = swapDetails.capabilties.minImageCount + 1;
    // define pointer to make code more readable
    const uint32_t *maxImageCount = &swapDetails.capabilties.maxImageCount;
    if (*maxImageCount > 0 && imageCount > *maxImageCount) {
        imageCount = *maxImageCount;
    }

    // create info
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    createInfo.surface = windowSurface; // glfw window surface
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = surfaceExtent;

    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // setup queue families
    QueueFamilyIndices indices = findDeviceQueue (physicalDevice);
    uint32_t queueFamilyIndices[QUEUE_COUNT] = {QUEUE_NAMES};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
    }

    createInfo.preTransform = swapDetails.capabilties.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // set present mode
    createInfo.presentMode = surfacePresentMode;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // create swapchain
    if (vkCreateSwapchainKHR (device, &createInfo, NULL, &swapchain.swapchain) != VK_SUCCESS) {
        debug_log ("Failed to create swapchain");
        return EXIT_FAILURE;
    }

    assert (swapchain.swapchain != NULL);

    // resize and create swapchainImageCount variable
    vkGetSwapchainImagesKHR (device, swapchain.swapchain, &swapchain.imageCount, NULL);
    swapchain.images = malloc (sizeof (VkImage) * swapchain.imageCount);
    vkGetSwapchainImagesKHR (device, swapchain.swapchain, &imageCount, &swapchain.images);


    // set swapchain variables
    swapchain.imageExtent = surfaceExtent;
    swapchain.imageFormat= surfaceFormat.format;

    return EXIT_SUCCESS;
}

// create image views
int createImageViews (void) {

    // resize swapchain image views to be same as swapchain images
    swapchain.imageViews = malloc (sizeof (VkImageView) * swapchain.imageCount);
    // createinfo pool
    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    // set rgba channels
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // setup basic subresource range (basic because no sterioscopic)
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    // create new image for each swapchain image
    for (uint32_t i = 0; i < swapchain.imageCount; i++) {

        imageViewCreateInfo.image = swapchain.images;
        if (vkCreateImageView (device, &imageViewCreateInfo, NULL, &swapchain.imageViews[i]) != VK_SUCCESS) {
            debug_log ("Failed to create VkImageView index #%i", i);
            return EXIT_FAILURE;
        }

    }
    return EXIT_SUCCESS;
}

//
// Helper function helper implementations
//

// function to check if a physical device meets requirements
uint16_t isDeviceSuitable (VkPhysicalDevice questionedDevice) {

    // get random device info
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties (questionedDevice, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures (questionedDevice, &deviceFeatures);
    
    uint16_t score = 0; // the device priority when selecting in future

    // requirment bools
    bool hasExtensionSupport = false; // if device support required extensions
    bool supportsQueues = false; // if device supports required queues
    bool supportsSwapchain = false; // if device supports adequate swapchain functionality

    // check if device supports required extensions
    if (checkDeviceExtensionSupport (questionedDevice) == EXIT_SUCCESS) { hasExtensionSupport = true; }

    // check if device support graphics queue and presentation support
    QueueFamilyIndices deviceQueue = {};
    deviceQueue = findDeviceQueue (questionedDevice);
    if ( deviceQueue.foundGraphicsFamily && deviceQueue.foundPresentFamily) { supportsQueues = true; }

    // check if device supports swapchain
    if (hasExtensionSupport) {
        SwapChainSupportDetails swapchainInfo = querySwapChainSupport (questionedDevice);
        supportsSwapchain = swapchainInfo.formatCount != 0 && swapchainInfo.presentModeCount != 0;
    }

    // DEBUG
    if (!supportsQueues)
        debug_log ("Lacks queue support");
    if (!hasExtensionSupport)
        debug_log ("Lacks extension support");
    if (!supportsSwapchain)
        debug_log ("Lacks swapchain support");

    // check if device supports all functionality
    if (hasExtensionSupport && supportsQueues && supportsSwapchain) {
        score = 1;
    }

    // score device based on type of gpu
    if (score == 0) {
        debug_log ("Device '%s' has score %i", deviceProperties.deviceName, score);
        return 0;

    } else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 4;
    } else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        // increase priority for integrated GPU if wanted
        if (preferIntegratedGPU) {
            score += 3;
        }
        score += 2;
    } else {
        score = 0;
    }

    //increase gpu score if same queue supports everything
    if (deviceQueue.graphicsFamily == deviceQueue.presentFamily) {
        score += 1;
    }
    return score;
}

// function to check if a physical device supports swapchain
int checkDeviceExtensionSupport (VkPhysicalDevice questionedDevice) {

    uint32_t availailableExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties (questionedDevice, NULL, &availailableExtensionCount, NULL);
    VkExtensionProperties availableExtensionNames[availailableExtensionCount];
    vkEnumerateDeviceExtensionProperties (questionedDevice, NULL, &availailableExtensionCount, availableExtensionNames);
    // highly efficient iterator (two means twice the speed)
    for (uint32_t i = 0; i < arrayLength(requiredDeviceExtensionNames); i++) {
        bool found = false; // if the current extension has been found
        for (uint32_t x = 0; x < availailableExtensionCount; x++) {
            if (strcmp (requiredDeviceExtensionNames[i], availableExtensionNames[i].extensionName) != 0) {
                found = true;
                break;
            }
        }
        // "handle" missing layer
        if (!found) {
            debug_log ("GPU does not support required extension: %s", requiredDeviceExtensionNames[i]);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

// function to check swapchain support
SwapChainSupportDetails querySwapChainSupport (VkPhysicalDevice questionedDevice) {
    SwapChainSupportDetails details = {};

    // check surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR (questionedDevice, windowSurface, &details.capabilties);

    // formats
    vkGetPhysicalDeviceSurfaceFormatsKHR (questionedDevice, windowSurface, &details.formatCount, NULL);
    assert (details.formatCount > 0);
    details.formats = malloc (sizeof (VkSurfaceFormatKHR) * details.formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR (questionedDevice, windowSurface, &details.formatCount, details.formats);
    assert (details.formats != NULL);

    if (details.formatCount == 0) { details.formats = NULL; }

    // present modes
    vkGetPhysicalDeviceSurfacePresentModesKHR (questionedDevice, windowSurface, &details.presentModeCount, NULL);
    assert (details.presentModeCount > 0);
    details.presentModes = malloc (sizeof (VkSurfaceFormatKHR) * details.presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR (questionedDevice, windowSurface, &details.presentModeCount, details.presentModes);
    assert (details.presentModes != NULL);

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat (uint32_t formatCount, const VkSurfaceFormatKHR *availableFormats) {

    for (uint32_t i = 0; i < formatCount; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode (uint32_t presentModeCount, const VkPresentModeKHR *availablePresentModes) {

    // return power saving present mode if using integrated GPU
    if (preferIntegratedGPU) { return VK_PRESENT_MODE_FIFO_KHR; }

    // check if prefered render mode is available
    for (uint32_t i = 0; i < presentModeCount; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent (VkSurfaceCapabilitiesKHR *capabilities) {

    // if (capabilities->currentExtent.width != UINT32_MAX) {
    //     return capabilities->currentExtent;
    // }

    int32_t width, height; // width and hight of swapchain buffer

    glfwGetFramebufferSize (window, &width, &height);

    // clamp dimensinos to fit swapchain capabilities
    width = clampValue (capabilities->minImageExtent.width, capabilities->maxImageExtent.width, width);
    height = clampValue (capabilities->minImageExtent.height, capabilities->maxImageExtent.height, height);

    VkExtent2D actualExtent = {
        (uint32_t) width,
        (uint32_t) height
    };

    return actualExtent;
}

// helper function to access required device queues
QueueFamilyIndices findDeviceQueue (VkPhysicalDevice questionedDevice) {

    QueueFamilyIndices indices = {};

    // get device queue properties
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties (questionedDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties deviceQueueProperties[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties (questionedDevice, &queueFamilyCount, deviceQueueProperties);

    // check for desired queue
    VkBool32 presentationSupport = false;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {

        // check for graphics queue support if not already found
        if (!indices.foundGraphicsFamily) {
            if (deviceQueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
                indices.foundGraphicsFamily = true;

            }
        }

        // check for presentationSupport if not already found
        if (!indices.foundPresentFamily) {
            if (vkGetPhysicalDeviceSurfaceSupportKHR (questionedDevice, i, windowSurface, &presentationSupport) != VK_SUCCESS)
                debug_log ("Failed find presentation support!");
            
            if (presentationSupport) {
                assert (presentationSupport);
                indices.foundPresentFamily = presentationSupport;
                indices.presentFamily = i;
            }
        }

        // check if all queues found
        if (indices.foundPresentFamily && indices.foundGraphicsFamily) {
            break;
        }
    }

    return indices;
}
// load debug messenger function
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
const VkAllocationCallbacks* pAllocator,
VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        debug_log ("Failed to load 'vkCreateDebugUtilsMessengerEXT', is NULL");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// function to destroy debug messenger
void DestroyDebugUtilsMessengerEXT ( VkInstance instance,
    VkDebugUtilsMessengerEXT *pDebugMessenger,
    const VkAllocationCallbacks *pAllocator) {
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr (instance, "vkDestroyUtilsMessengerEXT");

        if (func != NULL) {
            func (instance, debugMessenger, pAllocator);
        } else {
            debug_log ("Failed to load 'vkDestroyDebugUtilsMessengerEXT'");
        }
}
