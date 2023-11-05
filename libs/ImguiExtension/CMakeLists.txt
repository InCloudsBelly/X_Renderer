cmake_minimum_required(VERSION 3.7)
project(ImguiExtension)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/engine)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

file(GLOB_RECURSE IMGUI_SOURCE_FILES
        "*.cpp"
        "*.c"
        "*.h"
        "*.hpp"
        )

foreach(SOURCE_FILE IN ITEMS ${IMGUI_SOURCE_FILES})
    if (IS_ABSOLUTE "${SOURCE_FILE}")
        file(RELATIVE_PATH SOURCE_FILE_REL "${CMAKE_CURRENT_SOURCE_DIR}" "${SOURCE_FILE}")
    else()
        set(SOURCE_FILE_REL "${SOURCE_FILE}")
    endif()
    get_filename_component(SOURCE_PATH "${SOURCE_FILE_REL}" PATH)
    string(REPLACE "/" "\\" SOURCE_PATH_CONVERTED "${SOURCE_PATH}")
    source_group("${SOURCE_PATH_CONVERTED}" FILES "${SOURCE_FILE}")
endforeach()


# -ImGUI
# (compiles it as an static library)
set (IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../imgui")
file(GLOB IMGUI_SOURCES
   ${IMGUI_DIR}/*.cpp
   ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp 
   ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp 
)
add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(
   imgui
   PUBLIC
      ${IMGUI_DIR}
      ${IMGUI_DIR}/backends
      ${Vulkan_INCLUDE_DIRS}
      ${GLFW_INCLUDE_DIRS}
      ${SDL_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/../stb_image
      ${CMAKE_CURRENT_SOURCE_DIR}/../PerlinNoise
      ${CMAKE_CURRENT_SOURCE_DIR}/../VulkanMemoryAllocator/include

)    
target_link_libraries(
    imgui
PUBLIC
    SDL2-static
    volk::volk 
    volk::volk_headers
)

# We cam do this here, since the ImguiExtension dependency uses the engine and therefore is always included
# after the engine is added
add_library(${PROJECT_NAME} STATIC ${IMGUI_SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${X_ENGINE_COMPILE_DEFINITIONS})
target_link_libraries(${PROJECT_NAME} imgui )