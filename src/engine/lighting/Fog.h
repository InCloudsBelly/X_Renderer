#ifndef X_FOG_H
#define X_FOG_H

#include "../System.h"

namespace X {

    namespace Lighting {

        class Fog {

        public:
            Fog() = default;

            bool enable = true;

            vec3 color = vec3(0.5, 0.6, 0.7);

            float density = 0.05f;
            float height = 0.0f;
            float heightFalloff = 0.005f;

            float scatteringAnisotropy = 0.0f;


            bool renderVolume = true;
            bool useFroxelVolume = true;
            bool useRayMarchingVolume = false;

            vec3 froxelColor = vec3(1.0);
            bool froxelEnableJitter = true;
            bool froxelEnableTemperalAccumulating = true;

            float froxelFogAnisotropy = -0.3f;
            float froxelFogGlobalDensity = 0.5f;
            float froxelFogLightMul = 2.5f;

            float froxelHeightFogExponent = 0.0f;
            float froxelHeightFogOffset = 0.0f;
            float froxelHeightFogAmount = 0.0f;


        };

    }

}

#endif