#pragma once

#include "../../engine/src/ecs.hpp"

namespace realware
{
    namespace editor
    {
        class cRayHit
        {

        public:
            cRayHit(
                core::cApplication* app,
                core::cScene* scene,
                const std::vector<core::entity>& entities,
                const std::vector<core::entity>& excludeEntities,
                core::entity camera,
                const glm::vec2& mousePosition,
                const glm::vec2& windowSize,
                core::entity& resultEntity,
                core::boolean& resultBool,
                glm::vec3& result
            );
            ~cRayHit() {}

            static bool RayTriangleIntersection(
                const glm::vec3& orig,
                const glm::vec3& dir,
                const glm::vec3& v0,
                const glm::vec3& v1,
                const glm::vec3& v2,
                float& t,
                float& u,
                float& v
            );

        };
    }
}