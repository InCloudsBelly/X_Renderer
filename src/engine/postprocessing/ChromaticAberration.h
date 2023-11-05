#ifndef X_CHROMATICABBERATION_H
#define X_CHROMATICABBERATION_H

#include "../System.h"

namespace X {

    namespace PostProcessing {

        class ChromaticAberration {

        public:
            ChromaticAberration() = default;

            ///
            /// \param strength
            /// \param colorsReversed
            ChromaticAberration(float strength, bool colorsReversed = false) :
                    strength(strength), colorsReversed(colorsReversed), enable(true) {};

            bool enable = false;

            float strength = 1.0f;
            bool colorsReversed = false;

        };

    }

}


#endif