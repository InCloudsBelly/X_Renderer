#ifndef X_SYSTEM_H
#define X_SYSTEM_H

#define _CRT_SECURE_NO_WARNINGS

// There are three available defines to change some engine behavior:
// X_INSTANT_SHADER_RELOAD: Tracks shader changes and reloads them on change (high performance impact)
// X_SHOW_LOG: Prints out general information to the console or debug window (low performance impact)
// X_SHOW_API_DEBUG_LOG: Prints out graphics API specific things (medium performance impact)

#define X_SHOW_LOG
#define X_SHOW_API_DEBUG_LOG
#define X_INSTANT_SHADER_RELOAD

#include <stdint.h>
#include <string>
#include <memory>


#if defined(X_OS_ANDROID)

#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#define XLOG(...) __android_log_print(ANDROID_LOG_INFO, "X_LOG", __VA_ARGS__)

#elif defined(X_OS_WINDOWS) || defined(X_OS_LINUX) || defined(X_OS_MACOS)

#define XLOG(...) printf(__VA_ARGS__); printf("\n");

#endif

// SDL
#ifdef X_NO_APP
#define SDL_MAIN_HANDLED
#endif

// GLM
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/common.hpp>
#include <glm/gtx/transform.hpp>

#include "common/Ref.h"

namespace X {

    // Important definitions
    using glm::vec4;
    using glm::vec3;
    using glm::vec2;
    using glm::ivec2;
    using glm::ivec3;
    using glm::ivec4;

    using glm::mat4;
    using glm::mat3;
    using glm::mat4x3;
    using glm::mat3x4;

    typedef short float16;

}

#endif