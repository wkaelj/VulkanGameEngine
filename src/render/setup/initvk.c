// initvk implementation file

#include "initvk.h"

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

// struct to hold data to create shaders
typedef struct {

    uint32_t *pCode; // pointer to glsl code
    size_t codeSize; // size of pCode
    char *fileName; // name of the file for shader code
    char *shaderName; // name of shader
    VkShaderModule shaderModule; // shader module
    VkShaderStageFlagBits shaderType; // if shader is vert or frag shader
} ShaderCreateInfoContainer;

typedef struct {
    VkShaderModule *shaders;
    uint32_t shaderCount;
    VkPipelineShaderStageCreateInfo *createInfos;
} ShaderInfo;


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
VkPipeline pipeline; // vulkan pipeline
bool preferIntegratedGPU; // wether to prefer a integrated GPU over a discrete GPU
SveSwapchainData swapchain = {}; // store swapchain data
ShaderInfo shaderInfo = {}; // store shaders



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
int createWindow (SveVkInitInfo *info);
// create a vulkan instance and store it in instance
int createInstance (void);
// create a glfw window surface, and store it in windowSurface or smthn
int createWindowSurface (void);
// setup vulkan validation layers
int setupValidation (void); // always fails to load functions (function loaders broken this works)
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
// function to create render pass
int createRenderPass (void);
// create graphics pipeline
int createGraphicsPipeline (SveVkInitInfo *initInfo);
// load shader modules
int loadShaderModules (VkDevice *vulkanDevice, SveShaderModuleLoaderInfo *loaderInfo);
// destroy shader modules
int destroyShaderModules (void);
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
int sveInitVulkan (SveVkInitInfo *initInfo) {

    // manage struct inputs
    enableValidationLayers = initInfo->activateValidation;
    preferIntegratedGPU = initInfo->preferIntegratedGPU;

    // run init functions

    // create a glfw window
    if (createWindow (initInfo) != EXIT_SUCCESS) {
        debug_log ("Failed to create window.");
        return EXIT_FAILURE;
    }
    // setup validation layers
    if (enableValidationLayers) {
        if (setupValidation () != EXIT_SUCCESS) {
            enableValidationLayers = false;
            debug_log ("Incomplete validation layer support. Disabling validation layers.");
        }
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

    if (createRenderPass () != EXIT_SUCCESS) {
        debug_log ("Failed to create render pass.");
        return EXIT_FAILURE;
    }

    if (createGraphicsPipeline (initInfo) != EXIT_SUCCESS) {
        debug_log ("Failed to create render pipeline.");
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

int sveCleanVulkan (void) {

    vkDestroyPipeline (device, pipeline, NULL);
    vkDestroyPipelineLayout (device, pipelineLayout, NULL);
    vkDestroyRenderPass (device, renderPass, NULL);

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

//
// Private function implementation
// 

// function to create a glfw window
int createWindow (SveVkInitInfo *info) {

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

// function to initialize a vulkan instance
int createInstance (void) {

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.pApplicationInfo = &appInfo;

    // get glfw extensinos
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // include validation layer extensions
    const char **requiredExtensions;
    uint32_t requiredExtensionCount = 0;
    if (enableValidationLayers) {
        debug_log ("Validation Layers enabled");
        requiredExtensionCount = glfwExtensionCount + 1;
        requiredExtensions = malloc (sizeof (char *) * (requiredExtensionCount)); // allocate enough memory to fit glfwExtensionCount and 1 extra
        // set extensions to glfwExtensions
        // for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        //     requiredExtensions[i] = glfwExtensions[i];
        // }
        memcpy (requiredExtensions, glfwExtensions, sizeof (char *) * glfwExtensionCount);
        requiredExtensions[requiredExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    } else {
        requiredExtensionCount = glfwExtensionCount;
        requiredExtensions = glfwExtensions; // add no required extensions if not wanted
    }

    // check instance extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties (NULL, &extensionCount, NULL);
    VkExtensionProperties extensionNames[extensionCount];
    vkEnumerateInstanceExtensionProperties (NULL, &extensionCount, extensionNames);

    // check if required extensions are available
    for (uint32_t i = 0; i < requiredExtensionCount; i++) {

        bool found = false;

        //iterate through available extensions
        for (uint32_t x = 0; x < extensionCount; x++) {
            if (strcmp (requiredExtensions[i], extensionNames[x].extensionName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            debug_log ("Unavaible glfw extension: %s", requiredExtensions[i]);
            return EXIT_FAILURE;
        }
    }

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    // validation layers
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = arrayLength (requiredLayerNames);
        createInfo.ppEnabledLayerNames = requiredLayerNames;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance (&createInfo, NULL, &instance) != VK_SUCCESS) {
        debug_log ("Failed to create vulkan instance");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// debug callback function decleration
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

// function to setup optional debugging funcitonality
int setupValidation (void) {

    // check for validation support
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    // check if required extensions are available
    for (uint32_t i = 0; i < arrayLength (requiredLayerNames); i++) {
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
            debug_log ("Unavailable valiation layer: %s", requiredLayerNames[i]);
            return EXIT_FAILURE;
        }
    }

    // create debug messenger
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    if (CreateDebugUtilsMessengerEXT (instance, &createInfo, NULL, &debugMessenger) != VK_SUCCESS) {
        debug_log ("Failed to intialize debug messenger.");
        return EXIT_FAILURE;
    }
    debug_log ("Created Debug Messenger");
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

    uint8_t tmpDeviceOutput = 0; // used to store function output in for loop
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

// create render pass
int createRenderPass (void) {

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchain.imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // TODO implement stencil
    
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // attachment references
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

    // subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &colorAttachment;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass (device, &createInfo, NULL, &renderPass) != VK_SUCCESS) {
        debug_log ("Failed to create render pass");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// create grapics pipeline
int createGraphicsPipeline (SveVkInitInfo *initInfo) {

    // load shaders
    if (loadShaderModules (&device, initInfo->shaderLoaderInfo) != EXIT_SUCCESS) return EXIT_FAILURE;

    // fixed function pipline stages

    // input assembler
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = NULL; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = NULL; // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // make a triangle with each three verts
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE; // do not reuse verts

    // viewport
    VkViewport viewport = {};
    viewport.x = 0.0f; // viewport upper left corner
    viewport.y = 0.0f;
    viewport.width = (float) swapchain.imageExtent.width;
    viewport.height = (float) swapchain.imageExtent.height;
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = swapchain.imageExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;\

    // rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // dont render faces pointing backwards
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // front face is verts clockwise
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // fill tris

    // multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = NULL; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // color blending
    // framebuffer specific

    // enable later if I want special effects or smthn
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    // global color blending settings
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    // dynamic state (used to recreate pipline)
    //TODO dynamic state

    // pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = NULL;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = NULL;

    if (vkCreatePipelineLayout (device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
        debug_log ("Failed to create pipelineLayout.");
        return EXIT_FAILURE;
    }

    // create pipeline
    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    // set states from before
    createInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizer;
    createInfo.pMultisampleState = &multisampling;
    createInfo.pColorBlendState = &colorBlending;
    shaderInfo.shaders = shaderInfo.shaders;
    createInfo.stageCount = shaderInfo.shaderCount;
    createInfo.pStages = shaderInfo.createInfos;

    createInfo.layout = pipelineLayout;
    createInfo.renderPass = renderPass;
    createInfo.subpass = 0;

    // TODO use to create new pipelines
    // createInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = -1;

    // create graphics pipeline
    if (vkCreateGraphicsPipelines (device, VK_NULL_HANDLE, 1, &createInfo, NULL, &pipeline) != VK_SUCCESS) {
        debug_log ("Failed to create graphics pipeline");
        return EXIT_FAILURE;
    }

    // delete shader modules after creation
    destroyShaderModules ();
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
        supportsSwapchain = swapchainInfo.formats != NULL && swapchainInfo.presentModes != NULL;
    }

    // check if device supports all functionality
    if (hasExtensionSupport && supportsQueues && supportsSwapchain) {
        score = 1;
    }

    // score device based on type of gpu
    if (score == 0) {
        // do skip to return 0 if score 0;
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
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR (questionedDevice, windowSurface, &formatCount, details.formats);
    details.formatCount = formatCount;
    if (formatCount == 0) { details.formats = NULL; }

    // present modes
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR (questionedDevice, windowSurface, &presentModeCount, details.presentModes);
    details.presentModeCount = presentModeCount;
    if (presentModeCount == 0) { details.presentModes = NULL; }
    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat (uint32_t formatCount, const VkSurfaceFormatKHR *availableFormats) {

    for (uint32_t i = 0; i < formatCount; i++) {
        if (availableFormats->format == VK_FORMAT_B8G8R8_SRGB && availableFormats->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
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

// function to load shader modules
// load shader function reads list of shaders from SHADERLIST_FILEPATH, and then loads all of the shaders in to the array
int loadShaderModules (VkDevice *vulkanDevice, SveShaderModuleLoaderInfo *loaderInfo) {

    // read config file
    uint32_t shaderCount; // variable not to mess up shaderModule count
    if (readFileStringArray (loaderInfo->configFilePath, NULL, &shaderCount) != EXIT_SUCCESS) return EXIT_FAILURE;
    char *shaderConfigs[shaderCount];
    if (readFileStringArray (loaderInfo->configFilePath, shaderConfigs, &shaderCount) != EXIT_SUCCESS) return EXIT_FAILURE;

    // shader module create info
    ShaderCreateInfoContainer shaderModuleInfo[shaderCount]; // TODO check if this needs to be moved to a pointer
    const char argBreakChar = loaderInfo->argBreakChar;

    // parse file names
    for (size_t i = 0; i < shaderCount; i++) {
        // variables
        char *shaderParams[loaderInfo->paramCount];
        char *nameCopy = shaderConfigs[i]; // copy shaderconfigs into buffer var to be safe

        // splot config string apart at ARBREAK_CHAR
        for (size_t x = 0; x < loaderInfo->paramCount; x++) {
            shaderParams[x] = strsep (&nameCopy, &argBreakChar);
        }

        // read parameters to struct
        shaderModuleInfo[i].shaderName = shaderParams[0];
        shaderModuleInfo[i].fileName = shaderParams[2];

        // check if string is vertex or fragment, and chose vertex or fragment shader
        if (strcmp (shaderParams[1], "VERTEX") == 0) {
            shaderModuleInfo[i].shaderType = VK_SHADER_STAGE_VERTEX_BIT;
        } else if (strcmp (shaderParams[1], "FRAGMENT") == 0) {
            shaderModuleInfo[i].shaderType = VK_SHADER_STAGE_FRAGMENT_BIT;
        } else {
            debug_log ("Failed to read shader type of shader '%s', unrecognised param '%s'", shaderModuleInfo[i].shaderName, shaderParams[1]);
        }

        // debug
        assert (shaderModuleInfo[i].shaderName != NULL);
        assert (shaderModuleInfo[i].fileName != NULL);

        // read files
        if (readFileBinary (shaderModuleInfo[i].fileName, shaderModuleInfo[i].pCode, &shaderModuleInfo[i].codeSize) != EXIT_SUCCESS) {
            debug_log ("Failed to read shader from file '%s'", shaderConfigs[i]);
            return EXIT_FAILURE;
        }
    }

    // allocate memory to storage structs
    shaderInfo.createInfos = malloc (sizeof (VkPipelineShaderStageCreateInfo) * shaderCount);
    shaderInfo.shaders = malloc (sizeof (VkShaderModule) * shaderCount);
    shaderInfo.shaderCount = shaderCount;

    // create shader modules from file binaries
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {}; // define a shader module create info so it does not have to be redefined inside for loop
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    for (size_t i = 0; i < shaderCount; i++) {

        // define creatinfo properties
        shaderModuleCreateInfo.codeSize = shaderModuleInfo[i].codeSize; // filesize I read by readFileBinary
        shaderModuleCreateInfo.pCode = shaderModuleInfo[i].pCode; // file binary read by readFileBinary

        // create shader module, if failes throw error and exit failure
        if (vkCreateShaderModule (*vulkanDevice, &shaderModuleCreateInfo, NULL, &shaderModuleInfo[i].shaderModule) != VK_SUCCESS) {
            debug_log ("Failed to create shader module for shader file '%s'", shaderConfigs[i]);
            return EXIT_FAILURE;
        }

        // store shader module
        shaderInfo.shaders[i] = shaderModuleInfo[i].shaderModule;

        // create shader create info to use later
        VkPipelineShaderStageCreateInfo shaderModuleCreateInfo = {};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderModuleCreateInfo.module = shaderModuleInfo[i].shaderModule;
        shaderModuleCreateInfo.pName = shaderModuleInfo[i].shaderName;
        shaderModuleCreateInfo.pSpecializationInfo = NULL;
        shaderModuleCreateInfo.stage = VK_SHADER_STAGE_ALL_GRAPHICS;
        shaderModuleCreateInfo.flags = shaderModuleInfo[i].shaderType;

        shaderInfo.createInfos[i] = shaderModuleCreateInfo;
    }

    return EXIT_SUCCESS;
}

int destroyShaderModules (void) {

    // destroy shader modules
    for (uint32_t i = 0; i < shaderInfo.shaderCount; i++) {
        vkDestroyShaderModule (device, shaderInfo.shaders[i], NULL);
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
            vkGetPhysicalDeviceSurfaceSupportKHR (questionedDevice, i, windowSurface, &presentationSupport);
            if (presentationSupport) {
                indices.foundPresentFamily = true;
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
        }
}

// debug callback function to print debug messeges
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    debug_log ("Vulkan Error: %s", pCallbackData->pMessage);

    return VK_FALSE;
}