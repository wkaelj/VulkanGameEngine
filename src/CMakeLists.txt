
project (src VERSION 0.0)

add_library(${PROJECT_NAME} main.c)

add_subdirectory(render)
add_subdirectory(utilities)
add_subdirectory(debug)

target_link_directories(${PROJECT_NAME} PRIVATE
    render
    debug
    utilities
)

target_include_directories(${PROJECT_NAME} PRIVATE
    render
    debug
    utiltities
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    #Render
    Utilities
    Debug
)