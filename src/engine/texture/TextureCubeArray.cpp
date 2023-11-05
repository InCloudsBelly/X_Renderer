#include "TextureCubeArray.h"

#include "../loader/ImageLoader.h"

namespace X {

    namespace Texture {

        TextureCubeArray::TextureCubeArray(int32_t width, int32_t height, int32_t layers, VkFormat format,
            Wrapping wrapping, Filtering filtering) {

            this->format = format;
            Reallocate(Graphics::ImageType::ImageCubeArray, width, height, layers, filtering, wrapping);
            RecreateSampler(filtering, wrapping);

        }
    }

}