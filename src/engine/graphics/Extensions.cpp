#include "Extensions.h"
#include "Instance.h"

namespace X {

    namespace Graphics {

        std::unordered_set<std::string> Extensions::supportedExtensions;

        void Extensions::Process() {

            auto graphicsInstance = Instance::DefaultInstance;

            for (auto extension : graphicsInstance->extensionNames) {
                supportedExtensions.insert(std::string(extension));
            }

        }

        bool Extensions::IsSupported(const std::string& extension) {

            return supportedExtensions.find(extension) != supportedExtensions.end();

        }

    }

}