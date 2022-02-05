
project (src VERSION 0.0)

add_executable(${PROJECT_NAME} main.c)

add_subdirectory(render)
add_subdirectory(utilities)
add_subdirectory(debug)
add_subdirectory(libs)

target_link_directories(${PROJECT_NAME}
    PRIVATE render
    PRIVATE debug
    PRIVATE utilities
    PUBLIC libs
)

target_include_directories(${PROJECT_NAME}
    PUBLIC render
    PUBLIC debug
    PUBLIC utiltities
    PUBLIC libs
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    #Render
    Utilities
    Debug
)