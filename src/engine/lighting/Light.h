#ifndef X_ILIGHT_H
#define X_ILIGHT_H

#include "../System.h"
#include "Shadow.h"
#include "Volumetric.h"

#define X_STATIONARY_LIGHT 0
#define X_MOVABLE_LIGHT 1

#define X_DIRECTIONAL_LIGHT 0
#define X_POINT_LIGHT 1
#define X_SPOT_LIGHT 2

namespace X {

    namespace Lighting {

        class Light {

        public:
            virtual void RemoveShadow() = 0;

            virtual void RemoveVolumetric() = 0;

            virtual void Update(Camera* camera) = 0;

            inline Shadow* GetShadow() {
                return shadow;
            }

            inline Volumetric* GetVolumetric() {
                return volumetric;
            }

            int32_t type;
            int32_t mobility;

            vec3 color = vec3(1.0f);
            float intensity = 1.0f;

            std::string name;

        protected:
            Light(int32_t type, int32_t mobility) : type(type), mobility(mobility) {}

            Shadow* shadow = nullptr;
            Volumetric* volumetric = nullptr;

        };

    }

}


#endif
