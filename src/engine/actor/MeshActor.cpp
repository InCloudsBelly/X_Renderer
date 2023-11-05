#include "MeshActor.h"

namespace X {

    namespace Actor {

        void MeshActor::Update(Camera camera, float deltaTime,
            mat4 parentTransform, bool parentUpdate) {

            lastGlobalMatrix = globalMatrix;

            if (matrixChanged || parentUpdate) {

                if (!mesh.IsLoaded()) {
                    // Force re-evaluation until mesh is finally loaded
                    matrixChanged = true;
                    return;
                }

                matrixChanged = false;

                globalMatrix = parentTransform * GetMatrix();
                inverseGlobalMatrix = mat4x3(glm::inverse(globalMatrix));

                aabb = mesh->data.aabb.Transform(globalMatrix);
            }

        }

    }

}