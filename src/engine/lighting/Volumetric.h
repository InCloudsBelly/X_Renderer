#ifndef X_VOLUMETRIC_H
#define X_VOLUMETRIC_H

#include "../System.h"
#include "../RenderTarget.h"
#include "../texture/Texture2D.h"

namespace X {

    namespace Lighting {

        class Volumetric {

        public:
            Volumetric(int32_t sampleCount = 10, float intensity = 1.0f);

            int32_t sampleCount = 10;
            float intensity = 1.0f;

        };

    }

}

#endif