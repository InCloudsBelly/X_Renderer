#ifndef X_STATICMESHACTOR_H
#define X_STATICMESHACTOR_H

#include "MeshActor.h"

namespace X {

    namespace Actor {

        class StaticMeshActor : public MeshActor {

        public:
            StaticMeshActor() : MeshActor("",ResourceHandle<Mesh::Mesh>()) {}

            StaticMeshActor(ResourceHandle<Mesh::Mesh> mesh, mat4 matrix);

            void SetMatrix() {}

        };

    }

}

#endif