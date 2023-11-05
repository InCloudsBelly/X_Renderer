#ifndef X_MOVABLEMESHACTOR_H
#define X_MOVABLEMESHACTOR_H

#include "MeshActor.h"

namespace X {

    namespace Actor {

        class MovableMeshActor : public MeshActor {

        public:
            MovableMeshActor() : MeshActor("",ResourceHandle<Mesh::Mesh>()) {}

            explicit MovableMeshActor(std::string name ,ResourceHandle<Mesh::Mesh> mesh, mat4 matrix = mat4(1.0f))
                : MeshActor(name ,mesh) { this->SetMatrix(matrix); }

        };

    }

}

#endif