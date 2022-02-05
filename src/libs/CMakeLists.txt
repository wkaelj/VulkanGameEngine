
add_subdirectory(glfw)
add_subdirectory(Vulkan-Headers)

include_directory(
    glfw
    Vulkan-Headers
)

link_libraries(
    PUBLIC GLFW
    PUBLIC Vulkan-Headers
)