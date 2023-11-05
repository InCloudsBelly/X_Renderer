#include "DecalActor.h"

namespace X {

    namespace Actor {

        DecalActor::DecalActor(X::Decal *decal) : decal(decal) {



        }

        void DecalActor::Update(Camera, float deltaTime, 
            mat4 parentTransform, bool parentUpdate) {

            if (matrixChanged || parentUpdate) {

                matrixChanged = false;

                globalMatrix = parentTransform * GetMatrix();

                Volume::AABB base(vec3(-1.0f), vec3(1.0f));

                aabb = base.Transform(globalMatrix);

            }

        }

    }

}