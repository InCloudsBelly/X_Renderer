if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there")
endif()

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    cmake_policy(SET CMP0048 NEW)
    cmake_policy(SET CMP0074 NEW)
    cmake_policy(SET CMP0011 NEW)
    cmake_policy(SET CMP0042 NEW)
endif(COMMAND cmake_policy)

project(XRenderer VERSION 0.1.8)
cmake_minimum_required(VERSION 3.7)

# Only 64 bit is supported
###################################################################
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "32 bit isn't supported")
endif()

# Options and compiler settings ###################################################################
option(X_BUILD_SHARED "Force build as shared library" OFF)
option(X_OPENGL_ES "Use OpenGL ES instead of OpenGL" OFF)
option(X_EXPORT_MAIN "Add main file in root of directory tree. Add ${X_ENGINE_MAIN_FILE} to your executable" OFF)
option(X_NO_APP "Disables the engines main function" OFF)
option(X_DEMO "Build demo executable" OFF)
option(X_IMGUI "Activate ImGui integration" OFF)
option(X_ASSIMP "Activate Assimp integration" ON)

if (X_DEMO)
    set (X_IMGUI ON)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (CYGWIN OR MINGW)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -O3 -std=gnu++11" )
endif()

if (ANDROID)
    list(APPEND EXTRA_LIBS dl)
endif()

if (X_DEMO)
set(X_EXPORT_MAIN ON CACHE BOOL "Override engine settings" FORCE)
endif()

# Set dependencies location #######################################################################
set (Engine_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/engine)
set (DEMO_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/demo)
set (IMGUI_EXTENSION_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/ImguiExtension)

# Add dependencies ################################################################################
#find_package(SDL2 CONFIG REQUIRED)
find_package(Vulkan REQUIRED FATAL_ERROR)

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL)

if(X_ASSIMP)
    #find_package(assimp CONFIG REQUIRED)
    add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/libs/assimp)
endif()
if (ANDROID)
    add_subdirectory(${HIDAPI_LOCATION})
endif()

find_package(Threads REQUIRED)

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/libs/volk)
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/libs/VulkanMemoryAllocator)

set(ENABLE_OPT OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/libs/glslang)

set(SPIRV_REFLECT_EXECUTABLE OFF)
set(SPIRV_REFLECT_EXAMPLES   OFF)
set(SPIRV_REFLECT_STATIC_LIB   ON)
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/libs/SPIRV-Reflect)

set(SPIRV_TOOLS_BUILD_STATIC ON)
set(SPIRV_WERROR OFF)
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/libs/spirv-tools)

#find_package(volk CONFIG REQUIRED)
#find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
#find_package(glslang CONFIG REQUIRED)
#find_package(unofficial-spirv-reflect CONFIG REQUIRED)
#find_package(SPIRV-Tools-opt CONFIG REQUIRED)

add_subdirectory(${Engine_LOCATION})

add_subdirectory(${IMGUI_EXTENSION_LOCATION})

add_subdirectory(${DEMO_LOCATION})
