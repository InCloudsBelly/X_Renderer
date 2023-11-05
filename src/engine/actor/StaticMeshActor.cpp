#include "StaticMeshActor.h"

namespace X {

    namespace Actor {

        StaticMeshActor::StaticMeshActor(ResourceHandle<Mesh::Mesh> mesh, mat4 matrix) : MeshActor("",mesh) {

            Actor::SetMatrix(matrix);

        }

    }

}