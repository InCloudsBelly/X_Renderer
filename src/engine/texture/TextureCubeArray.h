#ifndef X_TEXTURECUBEARRAY_H
#define X_TEXTURECUBEARRAY_H

#include "../System.h"
#include "Texture.h"

namespace X {

    namespace Texture {

        class TextureCubeArray : public Texture {

        public:
            /**
             * Constructs a Texture2DArray object.
             */
            TextureCubeArray() = default;

            /**
              * Construct a Texture2DArray object.
              * @param width The width of the texture.
              * @param height The height of the texture.
              * @param layers The number of texture depth.
              * @param format The texture format.
              * @param wrapping The wrapping of the texture. Controls texture border behaviour.
              * @param filtering The filtering of the texture.
              */
            TextureCubeArray(int32_t width, int32_t height, int32_t layers, VkFormat format,
                Wrapping wrapping = Wrapping::Repeat, Filtering filtering = Filtering::Nearest);

        };

    }

}

#endif