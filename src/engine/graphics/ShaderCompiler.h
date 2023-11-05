#ifndef X_GRAPHICSSHADERCOMPILER_H
#define X_GRAPHICSSHADERCOMPILER_H

#include "Common.h"
#include "Shader.h"

#include <stdint.h>
#include <vector>

namespace X {

    namespace Graphics {

        class ShaderCompiler {

        public:
            static void Init();

            static void Shutdown();

            static std::vector<uint32_t> Compile(const ShaderStageFile& shaderFile,
                const std::vector<std::string>& macros, bool& success);

        };

    }

}

#endif
