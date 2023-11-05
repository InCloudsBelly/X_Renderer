#ifndef X_SSS_H
#define X_SSS_H

#include "../System.h"

namespace X {

    namespace Lighting {

        class SSS {

        public:
            SSS() = default;

            int sampleCount = 8;
            float maxLength = 0.5f;
            float thickness = 0.3f;

            bool enable = true;

        };

    }

}

#endif