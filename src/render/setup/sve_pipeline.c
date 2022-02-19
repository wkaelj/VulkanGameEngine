// function to create and manage graphics pipelines over the lifetime of the program
// automatically creates and destroys pipleines (soon)

#include "sve_pipeline.h"

//
// Struct definitions
//

// struct to hold data to create shaders
typedef struct {
    unsigned char *pCode; // pointer to glsl code
    size_t codeSize; // size of pCode
    char *fileName; // name of the file for shader code
    char *shaderName; // name of shader
    VkShaderModule shaderModule; // shader module
    VkShaderStageFlagBits shaderType; // if shader is vert or frag shader
} ShaderCreateInfoContainer;

// structure to store shaders
typedef struct {
    VkShaderModule *shaders;
    uint32_t shaderCount;
    VkPipelineShaderStageCreateInfo *createInfos;
} SveShaderInfo;

// used to store configuration structures that to not change often, during program runtime
typedef struct {
    VkGraphicsPipelineCreateInfo *defaultPipelineConfig;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkDevice sveDevice;
} SvePiplineVariables;

//
// Variable Declerations
//

VkPipeline pipeline; // Vulkan graphics pipline
SveShaderInfo shaderInfo;
SvePiplineVariables vars = {};

// initialized here to save memory in function
VkGraphicsPipelineCreateInfo customGraphicsPipelineCreateInfo = {};


//
// Private Function Declerations
//

// function to load shader modules, by reading a configuration file
int loadShaderModules (VkDevice vulkanDevice, SveShaderModuleLoaderInfo *loaderInfo);

// function to destroy shader modules after use
int destroyShaderModules (SveShaderInfo *pShaderInfo);

// function to create render pass
int createRenderPass (void);

// function to populate SvePiplineConfigInfo struct with the default config info
int sveDefaultPipelineConfig (SvePiplineConfigInfo *pConfigInfo);

//
// Public Functions
//

int sveCreateGraphicsPipeline (bool isCustomConfig, SvePiplineConfigInfo *pCustomConfig) {
        
    VkGraphicsPipelineCreateInfo *pipelineCreateInfo = malloc (sizeof (VkGraphicsPipelineCreateInfo));

    // check if custom config, otherwise use default
    if (isCustomConfig) {
       LOG_ERROR("NO CUSTOM >:(");
        pipelineCreateInfo[0] = *vars.defaultPipelineConfig;
    } else {
        pipelineCreateInfo[0] = *vars.defaultPipelineConfig;
    }

    // create graphics pipeline
    if (vkCreateGraphicsPipelines (vars.sveDevice, NULL, 1, pipelineCreateInfo, NULL, &pipeline) != SUCCESS) {
        LOG_ERROR("Failed to create graphics pipeline");
        return FAILURE;
    } else {
        LOG_DEBUG("Created graphics pipeline!!");
    }

    return SUCCESS;
}

// function to initialize the graphics pipline, and setup all
int sveInitGraphicsPipeline (SvePipelineCreateInfo *initInfo) {

    SvePiplineConfigInfo createInfo;

    sveDefaultPipelineConfig (&createInfo);

    vars.sveDevice = NULL;
    sveGetDevice (&vars.sveDevice);
    assert (vars.sveDevice != NULL);

    if (loadShaderModules (vars.sveDevice, initInfo->shaderLoaderInfo) != SUCCESS) return FAILURE;

    // call init functions
    createRenderPass (); // create render pass

    // populate default config info
    #define pipelineInfo vars.defaultPipelineConfig // I hate typing
    pipelineInfo = malloc (sizeof (VkGraphicsPipelineCreateInfo));

    // shader stages
    pipelineInfo->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo->stageCount = shaderInfo.shaderCount;
    pipelineInfo->pStages = shaderInfo.createInfos;

    // vertex input
    VkPipelineVertexInputStateCreateInfo vertexInfo = {};
    vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInfo.vertexAttributeDescriptionCount = 0;
    vertexInfo.vertexBindingDescriptionCount = 0;
    vertexInfo.pVertexAttributeDescriptions = NULL;
    vertexInfo.pVertexBindingDescriptions = NULL;

    // populate default configuration values
    pipelineInfo->pVertexInputState = &vertexInfo;
    pipelineInfo->pInputAssemblyState = &createInfo.inputAssemblyInfo;
    pipelineInfo->pViewportState = &createInfo.viewportInfo;
    pipelineInfo->pRasterizationState = &createInfo.rasterizationInfo;
    pipelineInfo->pMultisampleState = &createInfo.multisampleInfo;
    pipelineInfo->pDepthStencilState = &createInfo.depthStencilInfo;
    pipelineInfo->pDepthStencilState = &createInfo.depthStencilInfo;
    pipelineInfo->pColorBlendState = &createInfo.colorBlendInfo;

    pipelineInfo->layout = createInfo.pipelineLayout;
    pipelineInfo->renderPass = createInfo.renderPass;
    pipelineInfo->subpass = createInfo.subpass;

    pipelineInfo->basePipelineIndex = -1;
    pipelineInfo->basePipelineHandle = NULL;

    #undef pipelineInfo

    LOG_DEBUG("Created pipeline data");
    sveCreateGraphicsPipeline (false, NULL); // initialize default graphics pipeline
    
    return SUCCESS;
}

int sveCleanGraphicsPipeline (void) {

    // destroy all piplines
    vkDestroyPipeline (vars.sveDevice, pipeline, NULL);

    // destroy pipeline layout
    vkDestroyPipelineLayout (vars.sveDevice, vars.pipelineLayout, NULL);

    // free variables
    free (vars.defaultPipelineConfig);
    
    return SUCCESS;
}

//
// Private Functions
//

// the default piplinse configuration, should work for almost everything
int sveDefaultPipelineConfig (SvePiplineConfigInfo *pConfigInfo) {

    SvePiplineConfigInfo configInfo = {};

    uint32_t width, height;
    sveGetWindowSize (&width, &height);

    configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    configInfo.viewport.x = 0.0f;
    configInfo.viewport.y = 0.0f;
    configInfo.viewport.width = width;
    configInfo.viewport.height = height;
    configInfo.viewport.minDepth = 0.0f;
    configInfo.viewport.maxDepth = 1.0f;

    configInfo.scissor.offset.x = 0;
    configInfo.scissor.offset.y = 0;
    configInfo.scissor.extent.width = width;
    configInfo.scissor.extent.height = height;

    configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    configInfo.viewportInfo.viewportCount = 1;
    configInfo.viewportInfo.pViewports = &configInfo.viewport;
    configInfo.viewportInfo.scissorCount = 1;
    configInfo.viewportInfo.pScissors = &configInfo.scissor;

    configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
    configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    configInfo.rasterizationInfo.lineWidth = 1.0f;
    configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
    configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
    configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
    configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

    configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
    configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
    configInfo.multisampleInfo.pSampleMask = NULL;             // Optional
    configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
    configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

    configInfo.colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
    configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

    configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
    configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
    configInfo.colorBlendInfo.attachmentCount = 1;
    configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
    configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
    configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
    configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
    configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

    configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
    configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
    configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    // configInfo.depthStencilInfo.front = {};  // Optional
    // configInfo.depthStencilInfo.back = {};   // Optional

    *pConfigInfo = configInfo;

    return SUCCESS;
}

// create render pass
int createRenderPass (void) {

    SveSwapchainData swapchain = {};
    sveGetSwapchain (&swapchain);

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
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;

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

    if (vkCreateRenderPass (vars.sveDevice, &createInfo, NULL, &vars.renderPass) != VK_SUCCESS) {
        LOG_ERROR("Failed to create render pass");
        return FAILURE;
    }

    return SUCCESS;
}


// function to load shader modules
// load shader function reads list of shaders from SHADERLIST_FILEPATH, and then loads all of the shaders in to thModulee array
int loadShaderModules (VkDevice vulkanDevice, SveShaderModuleLoaderInfo *loaderInfo) {

    // read config file
    uint32_t shaderCount; // variable not to mess up shaderModule count
    if (readFileStringArray (loaderInfo->configFilePath, NULL, &shaderCount) != SUCCESS) return FAILURE;
    char *shaderConfigs[shaderCount];
    if (readFileStringArray (loaderInfo->configFilePath, shaderConfigs, &shaderCount) != SUCCESS) return FAILURE;

    // shader module create info
    ShaderCreateInfoContainer shaderModuleInfo[shaderCount];
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
            LOG_ERROR("Failed to read shader type of shader '%s', unrecognised param '%s'", shaderModuleInfo[i].shaderName, shaderParams[1]);
        }

        // debug
        assert (shaderModuleInfo[i].shaderName != NULL);
        assert (shaderModuleInfo[i].fileName != NULL);

        // read files
        if (readFileBinary (shaderModuleInfo[i].fileName, &shaderModuleInfo[i].pCode, &shaderModuleInfo[i].codeSize) != SUCCESS) {
            LOG_ERROR("Failed to read shader from file '%s'", shaderConfigs[i]);
            return FAILURE;
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
        shaderModuleCreateInfo.pCode = (const uint32_t *)shaderModuleInfo[i].pCode; // file binary read by readFileBinary

        // create shader module, if failes throw error and exit failure
        if (vkCreateShaderModule (vulkanDevice, &shaderModuleCreateInfo, NULL, &shaderModuleInfo[i].shaderModule) != VK_SUCCESS) {
            LOG_ERROR("Failed to create shader module for shader file '%s'", shaderConfigs[i]);
            return FAILURE;
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

    return SUCCESS;
}

// destroy shader module
int destroyShaderModules (SveShaderInfo *pShaderInfo) {

    VkDevice device;
    sveGetDevice (&device);

    // destroy shader modules
    for (uint32_t i = 0; i < pShaderInfo->shaderCount; i++) {
        vkDestroyShaderModule (device, pShaderInfo->shaders[i], NULL);
    }

    return SUCCESS;
}
