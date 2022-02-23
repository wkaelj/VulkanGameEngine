# VulkanGameEngine

VulkanGameEngine is a video game I am working on using the vulkan graphics api and the glfw window system.
I hope to use cglm as the matrix algebra library if I ever get that far

I am currently working on the fileutils and the pipeline creation, as of writing this.

# Build Intstructions
As of right now all you need installed is the Vulkan SDK (https://www.lunarg.com/vulkan-sdk/), and cmake (and make).

## CLONING
You need to do a clone ```--recursive``` or whatever so that your get the glfw submodule.
If you forgot use can do ```git submodule update --recursive```.

On Unix based systems, the cmake file should find the vulkan sdk and glfw libs all by itself.

On windows based systems, I got the cmakelists here -> https://youtu.be/ZuHK_5cJ6B8

1. Run the gencmake.sh script, or run cmake yourself/
2. Run the runmake.sh script, or run make yourself.
3. run the compile_shaders.sh script, to generate shaders, if you change the shaders
4. run the test.sh script, or run the SimpleVulkanEngine excecutable in the build folder.
