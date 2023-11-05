#include "VegetationActor.h"

namespace X {

    namespace Actor {

        VegetationActor::VegetationActor(const ResourceHandle<Mesh::Mesh>& mesh, vec3 position)
            : mesh(mesh), position(position) {



        }

        vec3 VegetationActor::GetPosition() const {

            return position;
        
        }

    }

}