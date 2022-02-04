cmake_minimum_required(VERSION 3.22.1)


#project name
project (SVE)

#add main as a executable
add_executable(${PROJECT_NAME} main.c)

add_subdirectory(setup)

target_include_directories(${PROJECT_NAME} PUBLIC setup)
target_link_directories(${PROJECT_NAME} PUBLIC setup)

target_link_libraries(${PROJECT_NAME} Setup)



