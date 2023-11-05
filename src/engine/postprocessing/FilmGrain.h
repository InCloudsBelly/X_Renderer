#ifndef X_FILMGRAIN_H
#define X_FILMGRAIN_H

#include "../System.h"

namespace X {

    namespace PostProcessing {

        class FilmGrain {

        public:
            FilmGrain() = default;

            FilmGrain(float strength) : strength(strength), enable(true) {};

            bool enable = false;

            float strength = 0.1f;
        };

    }

}


#endif