#ifndef X_MESHACTOR_H
#define X_MESHACTOR_H

#include "System.h"
#include "Actor.h"
#include "../mesh/Mesh.h"

namespace X {

    namespace Actor {

        class MeshActor : public Actor {

        public:
            void Update(Camera camera, float deltaTime,
                mat4 parentTransform, bool parentUpdate) override;

            ResourceHandle<Mesh::Mesh> mesh;

            mat4 lastGlobalMatrix = mat4(1.0);

        protected:
            explicit MeshActor(std::string name , const ResourceHandle<Mesh::Mesh>& mesh) : Actor(name),mesh(mesh) {}

        };


    }

}

#endif