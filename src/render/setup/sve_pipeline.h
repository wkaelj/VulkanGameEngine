#ifndef SVE_PIPLINE
#define SVE_PIPLINE

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include "../../debug/debug.h"
#include "../../utilities/fileutils.h"
#include "sve_device.h"

typedef struct {
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    uint32_t subpass;
} SvePiplineConfigInfo;

// structure to define information about shader config file
typedef struct {
    char *configFilePath;
    char argBreakChar;
    size_t paramCount;
} SveShaderModuleLoaderInfo;

// structure to hold pipline initialization info
typedef struct {
    SveShaderModuleLoaderInfo *shaderLoaderInfo;

} SvePipelineCreateInfo;

//
// Public Functions
//

// intitialize graphics piplisne functions and create a graphics pipline from default config
int sveInitGraphicsPipeline (SvePipelineCreateInfo *initInfo);

int sveCleanGraphicsPipeline (void);

int sveCreateGraphicsPipline (bool isCustomConfig, SvePiplineConfigInfo *pCustomConfig);


#endif