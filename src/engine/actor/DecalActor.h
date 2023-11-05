#ifndef X_DECALACTOR_H
#define X_DECALACTOR_H

#include "Actor.h"
#include "../Decal.h"

namespace X {

    namespace Actor {

        class DecalActor : public Actor {

        public:
            DecalActor() = default;

            explicit DecalActor(Decal* decal);

            void Update(Camera camera, float deltaTime,
                mat4 parentTransform, bool parentUpdate) override;

            Decal* decal = nullptr;

            vec4 color = vec4(1.0f);

        };

    }

}

#endif