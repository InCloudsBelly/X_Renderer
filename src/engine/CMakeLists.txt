if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there")
endif()

project(XEngine)
cmake_minimum_required(VERSION 3.7)
   
set(X_ENGINE_COMPILE_DEFINITIONS X_RENDERER_ENGINE)
# Validate options ################################################################################
if (X_NO_APP AND X_EXPORT_MAIN)
    set (X_EXPORT_MAIN OFF)
endif()

# Message options #################################################################################
if (X_EXPORT_MAIN)
    message(STATUS "Exporting main function")
endif()

if (X_NO_APP)
    message(STATUS "Building without app class")
    set(X_ENGINE_COMPILE_DEFINITIONS ${X_ENGINE_COMPILE_DEFINITIONS} X_NO_APP)
else()
    message(STATUS "Building with app class")
endif()

# Set built type macros ###########################################################################
if (DEFINED CMAKE_BUILD_TYPE)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(X_ENGINE_COMPILE_DEFINITIONS ${X_ENGINE_COMPILE_DEFINITIONS} X_BUILDTYPE_RELEASE)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        set(X_ENGINE_COMPILE_DEFINITIONS ${X_ENGINE_COMPILE_DEFINITIONS} X_BUILDTYPE_RELWITHDEBINFO)
    else()
        set(X_ENGINE_COMPILE_DEFINITIONS ${X_ENGINE_COMPILE_DEFINITIONS} X_BUILDTYPE_DEBUG)
    endif()
else()
    set(X_ENGINE_COMPILE_DEFINITIONS ${X_ENGINE_COMPILE_DEFINITIONS} X_BUILDTYPE_DEBUG)
endif()

# Configure platform specific stuff ###############################################################
if(WIN32)
    set(X_ENGINE_COMPILE_DEFINITIONS ${X_ENGINE_COMPILE_DEFINITIONS} X_OS_WINDOWS)

    set(X_ENGINE_LIBS assimp::assimp SDL2-static SPIRV-Tools-opt
            volk::volk volk::volk_headers VulkanMemoryAllocator
            spirv-reflect-static glslang SPIRV)
endif()

if (MINGW)
    set(X_ENGINE_LIBS mingw32 ${X_ENGINE_LIBS})
endif()

if(APPLE)
    set(X_ENGINE_COMPILE_DEFINITIONS ${X_ENGINE_COMPILE_DEFINITIONS} X_OS_MACOS)

    set(X_ENGINE_LIBS assimp::assimp SDL2-static
            volk::volk volk::volk_headers SPIRV-Tools-opt
            unofficial::vulkan-memory-allocator::vulkan-memory-allocator
            unofficial::spirv-reflect::spirv-reflect glslang::SPIRV)
endif()

if(ANDROID)
    set(X_ENGINE_COMPILE_DEFINITIONS ${X_ENGINE_COMPILE_DEFINITIONS} X_OS_ANDROID)

    set(X_ENGINE_LIBS assimp::assimp SDL2-static hidapi)
    set(X_ENGINE_SYSTEM_LIBS android m)
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    set(X_ENGINE_COMPILE_DEFINITIONS ${X_ENGINE_COMPILE_DEFINITIONS} X_OS_LINUX)
    
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")

    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "./")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    
    set(X_ENGINE_LIBS assimp::assimp SDL2-static
            volk::volk volk::volk_headers unofficial::vulkan-memory-allocator::vulkan-memory-allocator
            unofficial::spirv-reflect::spirv-reflect SPIRV-Tools-opt glslang::SPIRV)
endif()

# Add source files ################################################################################
file(GLOB_RECURSE X_ENGINE_SOURCE_FILES
        "*.cpp"
        "*.c"
        "*.h"
        "*.hpp"
        )

# Grouping source files into tree structure #######################################################
foreach(SOURCE_FILE IN ITEMS ${X_ENGINE_SOURCE_FILES})
    if (IS_ABSOLUTE "${SOURCE_FILE}")
        file(RELATIVE_PATH SOURCE_FILE_REL "${CMAKE_CURRENT_SOURCE_DIR}" "${SOURCE_FILE}")
    else()
        set(SOURCE_FILE_REL "${SOURCE_FILE}")
    endif()
    get_filename_component(SOURCE_PATH "${SOURCE_FILE_REL}" PATH)
    string(REPLACE "/" "\\" SOURCE_PATH_CONVERTED "${SOURCE_PATH}")
    source_group("${SOURCE_PATH_CONVERTED}" FILES "${SOURCE_FILE}")
endforeach()   
    
if (X_OPENGL_ES)
set(X_ENGINE_COMPILE_DEFINITIONS ${X_ENGINE_COMPILE_DEFINITIONS} X_API_GLES CACHE INTERNAL "")
else()
set(X_ENGINE_COMPILE_DEFINITIONS ${X_ENGINE_COMPILE_DEFINITIONS} X_API_GL CACHE INTERNAL "")
endif()

if (X_EXPORT_MAIN)
set(X_ENGINE_MAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp CACHE INTERNAL "Make main global")
list(FILTER X_ENGINE_SOURCE_FILES EXCLUDE REGEX "(^.*\/|^)Main\.cpp$")
endif()

if (NOT X_ASSIMP)
list(FILTER X_ENGINE_SOURCE_FILES EXCLUDE REGEX "(^.*\/|^)ModelLoader\.(c(pp)?|h(pp)?)$")
endif()

# Configure linking ###############################################################################
if(ANDROID)    
    add_library(${PROJECT_NAME} SHARED ${X_ENGINE_SOURCE_FILES})
else()
    list(FILTER X_ENGINE_SOURCE_FILES EXCLUDE REGEX "(^.*\/|^)[Aa]pp\.(c(pp)?|h(pp)?)$")
if (X_NO_APP)
    list(FILTER X_ENGINE_SOURCE_FILES EXCLUDE REGEX "(^.*\/|^)Main\.(c(pp)?|h(pp)?)$")
endif()
if (X_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${X_ENGINE_SOURCE_FILES})
else()
    add_library(${PROJECT_NAME} STATIC ${X_ENGINE_SOURCE_FILES})
endif()
endif()

find_path(GLM_INCLUDE_DIRS "glm/common.hpp")
find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")

# Include directories and definitions #############################################################
target_compile_definitions(${PROJECT_NAME} PUBLIC ${X_ENGINE_COMPILE_DEFINITIONS})
target_include_directories(${PROJECT_NAME} 
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/stb_image
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/VulkanMemoryAllocator/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/yaml-cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/assimp
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs)
    
# Link libraries ##################################################################################
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${X_ENGINE_LIBS} LINK_PRIVATE ${X_ENGINE_SYSTEM_LIBS})
