#ifndef X_SKY_H
#define X_SKY_H

#include "../System.h"

#include "EnvironmentProbe.h"
#include "DirectionalLight.h"
#include "Atmosphere.h"
#include "VolumetricClouds.h"

namespace X {

    namespace Lighting {

        class Sky {

        public:
            Sky();

            EnvironmentProbe* GetProbe();

            vec3 planetCenter = vec3(0.0f, -650000.0f, 0.0f);
            float planetRadius = 650000.0f;

            Ref<DirectionalLight> sun = nullptr;
            Ref<Atmosphere> atmosphere = nullptr;
            Ref<VolumetricClouds> clouds = nullptr;

            Ref<EnvironmentProbe> probe = nullptr;

        };

    }

}


#endif