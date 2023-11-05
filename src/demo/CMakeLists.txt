cmake_minimum_required(VERSION 3.7)
project(XRendererDemo)

# Note: For this project, the root CMakeLists.txt turns
# the X_IMGUI and X_EXPORT_MAIN options on.

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

file(GLOB_RECURSE DEMO_SOURCE_FILES
        "*.cpp"
        "*.c"
        "*.h"
        "*.hpp"
        )

# Required: Set both the source and dependency directories 
# as include directories
include_directories(../engine)
include_directories(../../libs)

foreach(SOURCE_FILE IN ITEMS ${DEMO_SOURCE_FILES})
    if (IS_ABSOLUTE "${SOURCE_FILE}")
        file(RELATIVE_PATH SOURCE_FILE_REL "${CMAKE_CURRENT_SOURCE_DIR}" "${SOURCE_FILE}")
    else()
        set(SOURCE_FILE_REL "${SOURCE_FILE}")
    endif()
    get_filename_component(SOURCE_PATH "${SOURCE_FILE_REL}" PATH)
    string(REPLACE "/" "\\" SOURCE_PATH_CONVERTED "${SOURCE_PATH}")
    source_group("${SOURCE_PATH_CONVERTED}" FILES "${SOURCE_FILE}")
endforeach()  

# We want to make sure that the linker searches for local libraries first
if (UNIX AND NOT APPLE AND NOT ANDROID)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
endif()
# We use the exported main file from the XEngine library to able to use
# the app class. Alternatively, you can write a main function yourself.
# To export the main file the X_EXPORT_MAIN option has to be turned on.
add_executable(${PROJECT_NAME} ${DEMO_SOURCE_FILES} ${X_ENGINE_MAIN_FILE})
# Required: Add the compile definitions of the library, such that includes work properly
target_compile_definitions(${PROJECT_NAME} PUBLIC ${X_ENGINE_COMPILE_DEFINITIONS})
# We want to use both ImGui and the XEngine. For ImGui, the X_IMGUI option
# needs to be turned on.
target_link_libraries (${PROJECT_NAME} XEngine ImguiExtension)