#ifndef X_SHARPEN_H
#define X_SHARPEN_H

#include "../System.h"

namespace X {

    namespace PostProcessing {

        class Sharpen {

        public:
            Sharpen() = default;

            Sharpen(float factor) : enable(true), factor(factor) {}

            bool enable = false;
            float factor = 0.25f;

        };

    }

}

#endif