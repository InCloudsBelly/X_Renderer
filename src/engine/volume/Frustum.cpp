#include "Frustum.h"

namespace X {

    namespace Volume {

        Frustum::Frustum(const std::vector<vec3>& corners) {

            Resize(corners);

        }

        Frustum::Frustum(mat4 matrix) {

            Resize(matrix);

        }

        void Frustum::Resize(std::vector<vec3> corners) {

            this->corners = corners;
            planes[NEAR_PLANE] = Plane(corners[4], corners[5], corners[7]);
            planes[FAR_PLANE] = Plane(corners[1], corners[0], corners[2]);
            planes[TOP_PLANE] = Plane(corners[5], corners[4], corners[0]);
            planes[BOTTOM_PLANE] = Plane(corners[6], corners[7], corners[3]);
            planes[RIGHT_PLANE] = Plane(corners[7], corners[5], corners[3]);
            planes[LEFT_PLANE] = Plane(corners[4], corners[6], corners[2]);

        }

        void Frustum::Resize(mat4 matrix) {

            CalculateCorners(matrix);
            Resize(corners);

        }

        bool Frustum::Intersects(AABB aabb) {
            vec3 s[8] = {
                        {aabb.max.x, aabb.max.y, aabb.min.z},
                        {aabb.max.x, aabb.max.y, aabb.max.z},
                        {aabb.max.x, aabb.min.y, aabb.min.z},
                        {aabb.max.x, aabb.min.y, aabb.max.z},
                        {aabb.min.x, aabb.max.y, aabb.min.z},
                        {aabb.min.x, aabb.max.y, aabb.max.z},
                        {aabb.min.x, aabb.min.y, aabb.min.z},
                        {aabb.min.x, aabb.min.y, aabb.max.z},
            };

            ////check any Frustum corner in AABB
            //for (int j = 0; j < 8; ++j)
            //{
            //    bool pointInAllPlane = true;
            //    for (uint8_t i = 0; i < 6; i++) {

            //        auto& normal = planes[i].normal;
            //        auto& distance = planes[i].distance;

            //        if (distance + glm::dot(normal, s[j]) > 0.0f)
            //            pointInAllPlane =  false;
            //    }
            //    if (pointInAllPlane)
            //    {
            //        return true;
            //    }
            //}

            ////check any Frustum corner in AABB
            //for (int j = 0; j < 8; ++j)
            //{
            //    bool pointInAllPlane = true;
            //    vec3& corner = corners[j];
            //    if(! (corner.x > aabb.min.x && corner.x < aabb.max.x))
            //        pointInAllPlane = false;
            //    if (!(corner.y > aabb.min.y && corner.y < aabb.max.y))
            //        pointInAllPlane = false;
            //    if (!(corner.y > aabb.min.y && corner.y < aabb.max.y))
            //        pointInAllPlane = false;
            //    if (pointInAllPlane)
            //    {
            //        return true;
            //    }
            //}
           
            
            bool intersects = true;
            for (uint8_t i = 0; i < 6; i++) {
                bool is_any_vertex_on_positive_side = false;

                for (int j = 0; j < 8; ++j)
                {
                    auto& normal = planes[i].normal;
                    auto& distance = planes[i].distance;

                    if (distance + glm::dot(normal, s[j]) < 0.0f)
                    {
                        is_any_vertex_on_positive_side = true;
                        break;
                    }
                }
                bool all_vertex_on_negative_side = !is_any_vertex_on_positive_side;
                if (all_vertex_on_negative_side) {
                    intersects = false;
                    break;
                }
            }

            if (!intersects) {
                return intersects;
            }

            //check any Frustum corner in AABB
         /*   {
                bool is_any_vertex_on_positive_side = false;
                for (int j = 0; j < 8; ++j)
                {
                    vec3& corner = corners[j];
                    if (corner.x > aabb.min.x)
                    {
                        is_any_vertex_on_positive_side = true;
                        break;
                    }
                }
                bool all_vertex_on_negative_side = !is_any_vertex_on_positive_side;
                if (all_vertex_on_negative_side) {
                    intersects = false;
                    return intersects;
                }
            }*/
            return intersects;

        }

        bool Frustum::IsInside(AABB aabb) {

            for (uint8_t i = 0; i < 6; i++) {

                auto& normal = planes[i].normal;
                auto& distance = planes[i].distance;

                vec3 s;
                s.x = normal.x >= 0.0f ? aabb.min.x : aabb.max.x;
                s.y = normal.y >= 0.0f ? aabb.min.y : aabb.max.y;
                s.z = normal.z >= 0.0f ? aabb.min.z : aabb.max.z;

                if (distance + glm::dot(normal, s) < 0.0f)
                    return false;

            }

            return true;

        }

        std::vector<vec4> Frustum::GetPlanes() {

            std::vector<vec4> planes;

            for (uint8_t i = 0; i < 6; i++) {
                planes.push_back(vec4(this->planes[i].normal,
                    this->planes[i].distance));
            }

            return planes;

        }

        std::vector<vec3> Frustum::GetCorners() {

            return corners;

        }

        void Frustum::CalculateCorners(mat4 matrix) {

            // Somehow far and near points are reversed
            vec3 vectors[8] = {
                vec3(1.0f, -1.0f, -1.0f),
                vec3(-1.0f, -1.0f, -1.0f),
                vec3(1.0f, 1.0f, -1.0f),
                vec3(-1.0f, 1.0f, -1.0f),
                vec3(1.0f, -1.0f, 1.0f),
                vec3(-1.0f, -1.0f, 1.0f),
                vec3(1.0f, 1.0f, 1.0f),
                vec3(-1.0f, 1.0f, 1.0f)
            };

            corners.clear();
            auto inverseMatrix = glm::inverse(matrix);

            for (uint8_t i = 0; i < 8; i++) {
                auto homogenous = inverseMatrix * vec4(vectors[i], 1.0f);
                corners.push_back(vec3(homogenous) / homogenous.w);
            }

        }

    }

}