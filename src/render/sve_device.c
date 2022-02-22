// initvk implementation file

#include "sve_device.h"
// #include <string.h> // included in sve_header

//
// Magic Number Definitions
//



//
// Struct definitions & implementations
//

// structure to manage rendering device candidates
typedef struct {
    VkPhysicalDevice physicalDevice;
    uint16_t deviceScore;
} SuitableDevice;

//
// Variable Declerations
//

VkInstance instance; // vulkan instance
VkPhysicalDevice physicalDevice; // vulkan rendering device
VkDevice device; // logical vulkan device
VkQueue graphicsQueue; // vulkan graphics queue
VkQueue presentQueue; // vulkan presentation queue
VkDebugUtilsMessengerEXT debugMessenger; // vulkan debug messenger
VkPipelineLayout pipelineLayout; // vulkan pipeline layout
bool preferIntegratedGPU; // wether to prefer a integrated GPU over a discrete GPU

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

// create a vulkan instance and store it in instance
int createInstance (void);
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

SwapChainSupportDetails querySwapChainSupport (VkPhysicalDevice vulkanDevice);




// function loaders
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
    if (sveCreateWindow (initInfo->windowWidth, initInfo->windowHeight, initInfo->windowName, initInfo->windowResizable) != SUCCESS) {
        LOG_ERROR("Failed to create window.");
        return FAILURE;
    }
    if (createInstance () != EXIT_SUCCESS) {
        LOG_ERROR("Failed to create vulkan instance.");
        return EXIT_FAILURE;
    }

    if (sveCreateWindowSurface (instance) != SUCCESS) {
        LOG_ERROR("Failed to create window surface");
        return FAILURE;
    }
    // create window surface
    // if (sveCreateWindowSurface (instance) != EXIT_SUCCESS) {
    //     LOG_ERROR("Failed to create GLFW window surface");
    // }
    // select gpu
    if (selectPhysicalDevice () != EXIT_SUCCESS) {
        LOG_ERROR("Failed to select a physical device.");
        return EXIT_FAILURE;
    }
    // create a device
    if (createLogicalDevice () != EXIT_SUCCESS) {
        LOG_ERROR("Failed to create logical device.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// get swapchain support details
SwapChainSupportDetails *sveGetSwapchainSupport (VkPhysicalDevice optionalDevice) {

    if (optionalDevice != NULL) {
        SwapChainSupportDetails swapchainDetails = querySwapChainSupport (optionalDevice);
    }

    // if device has not been created, why?
    if (physicalDevice == VK_NULL_HANDLE) {
        LOG_INFO ("sveGetSwapchainSupport called before phyical device selection");
        return NULL;
    }

    SwapChainSupportDetails *boop = malloc (sizeof (SwapChainSupportDetails));
    *boop = querySwapChainSupport (physicalDevice);
    return boop;
    // FIXME return is invalid

    // try and detect if swapchain support has already been checked
    static SwapChainSupportDetails *out;
    if (out == NULL) {
        SwapChainSupportDetails swapchainDetails = querySwapChainSupport (physicalDevice);
        out = &swapchainDetails;

    }

    return out;
}

// returns graphics queue families
QueueFamilyIndices *sveGetQueueFamilies (VkPhysicalDevice optionalDevice) {
   
   
    if (optionalDevice != NULL) {
        QueueFamilyIndices indices = findDeviceQueue (optionalDevice);
    }

    // if device has not been created, why?
    if (physicalDevice == VK_NULL_HANDLE) {
        return NULL;
    }

    // try and detect if swapchain support has already been checked
    static QueueFamilyIndices *out;
    if (out == NULL) {
        QueueFamilyIndices indices = findDeviceQueue (physicalDevice);
        out = &indices;

    }

    return out;
}

int sveDestroyDevice (void) {

    vkDestroyDevice (device, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT  (instance, &debugMessenger, NULL);
    }

    vkDestroySurfaceKHR (instance, sveGetWindowSurface (), NULL);

    DestroyDebugUtilsMessengerEXT(instance, &debugMessenger, NULL);

    sveDestroyWindowSurface (instance);
    vkDestroyInstance (instance, NULL);

    sveDestroyWindow ();

    return EXIT_SUCCESS;
}

VkDevice sveGetDevice (void) {
    assert (device != NULL);

    return device;
}

//
// Private function implementation
// 


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
                LOG_DEBUG("Found layer: %s", availableLayers[x].layerName);
                found = true;
                break;
            }
        }

        if (found == false) {
            LOG_DEBUG("Unavailable valiation layer: %s", requiredLayerNames[i]);
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
    LOG_VALIDATION("Validation Layer: %s", pCallbackData->pMessage);

    return VK_FALSE;
}
// function to initialize a vulkan instance
int createInstance (void) {
    
    // check if validation support is available
    if (enableValidationLayers && checkValidationLayerSupport() != EXIT_SUCCESS) {
        LOG_DEBUG("validation layers requested, but not available!");
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
    uint32_t windowExtensionCount = 0;
    char **windowExtensions;
    sveGetRequiredWindowExtensions (&windowExtensions, &windowExtensionCount);

    int32_t extensionCount = windowExtensionCount + 1;
    const char* extensions[extensionCount];
    for (int i = 0; i < windowExtensionCount; i++)
    {
        extensions[i] = windowExtensions[i];
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
        LOG_ERROR("Failed to create vulkan instance");
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
        LOG_FATAL("No GPU detected.");
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
        LOG_FATAL("No suitable GPU detected.");
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
    uint32_t queueFamilies[QUEUE_COUNT] = QUEUE_NAMES(indices); // queue indices
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

// function to check swapchain support
SwapChainSupportDetails querySwapChainSupport (VkPhysicalDevice questionedDevice) {
    SwapChainSupportDetails details = {};

    // check surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR (questionedDevice, sveGetWindowSurface (), &details.capabilties);

    // formats
    vkGetPhysicalDeviceSurfaceFormatsKHR (questionedDevice, sveGetWindowSurface (), &details.formatCount, NULL);
    assert (details.formatCount > 0);
    details.formats = malloc (sizeof (VkSurfaceFormatKHR) * details.formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR (questionedDevice, sveGetWindowSurface (), &details.formatCount, details.formats);
    assert (details.formats != NULL);

    if (details.formatCount == 0) { details.formats = NULL; }

    // present modes
    vkGetPhysicalDeviceSurfacePresentModesKHR (questionedDevice, sveGetWindowSurface (), &details.presentModeCount, NULL);
    assert (details.presentModeCount > 0);
    details.presentModes = malloc (sizeof (VkSurfaceFormatKHR) * details.presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR (questionedDevice, sveGetWindowSurface (), &details.presentModeCount, details.presentModes);
    assert (details.presentModes != NULL);

    return details;
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

    // check if device has swapchain support
    SwapChainSupportDetails swapchainDetails = querySwapChainSupport (questionedDevice);
    supportsSwapchain = swapchainDetails.formatCount > 0 && swapchainDetails.presentModeCount > 0;


    // DEBUG
    if (!supportsQueues)
        LOG_ERROR("Device '%s'Lacks queue support", deviceProperties.deviceName);
    if (!hasExtensionSupport)
        LOG_ERROR("Lacks '%s' extension support", deviceProperties.deviceName);
    if (!supportsSwapchain)
        LOG_ERROR("Lacks '%s' swapchain support", deviceProperties.deviceName);

    // check if device supports all functionality
    if (hasExtensionSupport && supportsQueues && supportsSwapchain) {
        score = 1;
    }

    // score device based on type of gpu
    if (score == 0) {
        LOG_INFO("Device '%s' has score %i", deviceProperties.deviceName, score);
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
            LOG_FATAL("GPU does not support required extension: %s", requiredDeviceExtensionNames[i]);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
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
            if (vkGetPhysicalDeviceSurfaceSupportKHR (questionedDevice, i, sveGetWindowSurface (), &presentationSupport) != VK_SUCCESS)
                LOG_INFO("Failed find presentation support!");
            
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
        LOG_VALIDATION("Failed to load 'vkCreateDebugUtilsMessengerEXT', is NULL");
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
            LOG_VALIDATION("Failed to load 'vkDestroyDebugUtilsMessengerEXT'");
        }
}
