#ifndef X_ATMOSPHERE_H
#define X_ATMOSPHERE_H

#include "../System.h"

#include "EnvironmentProbe.h"

namespace X {

    namespace Lighting {

        class Atmosphere {
        public:
            Atmosphere(float height = 100000.0f, int32_t probeResolution = 128);

            float height = 100000.0f;

            EnvironmentProbe probe;

        };

    }

}


#endif