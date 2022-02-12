#!/bin/bash

$VULKAN_SDK/bin/glslc Config/shaders/simple_shader.vert -o build/Config/shaders/simple_shader.vert.spv
$VULKAN_SDK/bin/glslc Config/shaders/simple_shader.frag -o build/Config/shaders/simple_shader.frag.spv