#include <iostream>
#include "ray_hit.hpp"
#include "../../engine/src/render_manager.hpp"

namespace realware
{
    namespace editor
    {
        cRayHit::cRayHit(
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
        )
        {
            core::sCCamera* cameraComponent = scene->Get<core::sCCamera>(camera);
            core::sCTransform* cameraTransformComponent = scene->Get<core::sCTransform>(camera);

            // Convert mouse position to world position
            float x = (2.0f * (mousePosition.x / windowSize.x)) - 1.0f;
            float y = (2.0f * ((windowSize.y - mousePosition.y) / windowSize.y)) - 1.0f;
            float z = -1.0f;
            glm::vec3 rayNds = glm::vec3(x, y, z);
            glm::vec4 rayClip = glm::vec4(rayNds, 1.0f);
            glm::vec4 rayEye = glm::inverse(cameraComponent->Projection) * rayClip;
            rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
            glm::vec3 rayWorld = glm::vec3(glm::inverse(cameraComponent->View) * rayEye);
            rayWorld = glm::normalize(rayWorld);

            // Create ray
            glm::vec3 rayOrigin = glm::vec3(
                cameraTransformComponent->Position.x,
                cameraTransformComponent->Position.y,
                cameraTransformComponent->Position.z
            );
            glm::vec3 rayDirection = glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z);

            // Run through all geometry triangles and test for ray hit
            float t = FLT_MAX * 0.5f;
            glm::vec3 p = glm::vec3(0.0f);
            resultBool = core::K_FALSE;
            scene->ForEach<core::sCGeometry>(
                app,
                [rayOrigin, rayDirection, &excludeEntities, &t, &p, &resultEntity, &resultBool]
                (core::cApplication* app_, core::cScene* scene_, core::sCGeometry* geometry_)
                {
                    for (auto& entity : excludeEntities)
                    {
                        if (entity == geometry_->Owner) {
                            return;
                        }
                    }

                    if (geometry_->Geometry == nullptr) {
                        return;
                    }

                    core::sCTransform* transform = scene_->Get<core::sCTransform>(geometry_->Owner);
                    render::sVertex* vertices = (render::sVertex*)geometry_->Geometry->VertexPtr;
                    render::index* indices = (render::index*)geometry_->Geometry->IndexPtr;

                    for (core::s32 i = 0; i < geometry_->Geometry->IndexCount; i += 3)
                    {
                        glm::vec3 v0 = transform->World *
                            glm::vec4(vertices[indices[i]].Position, 1.0f);
                        glm::vec3 v1 = transform->World *
                            glm::vec4(vertices[indices[i + 1]].Position, 1.0f);
                        glm::vec3 v2 = transform->World *
                            glm::vec4(vertices[indices[i + 2]].Position, 1.0f);

                        float distance = 0.0f;
                        float u = 0.0f, v = 0.0f;
                        bool hit = RayTriangleIntersection(
                            rayOrigin,
                            rayDirection,
                            v0,
                            v1,
                            v2,
                            distance,
                            u,
                            v
                        );

                        if (hit && distance < t && distance > 0.0f)
                        {
                            resultEntity = geometry_->Owner;
                            resultBool = core::K_TRUE;
                            t = distance;
                        }
                    }
                }
            );

            // Check with large invisible floor triangle
            glm::vec3 v0 = glm::vec3(-9999.0f, 0.0f, 9999.0f);
            glm::vec3 v1 = glm::vec3(0.0f, 0.0f, -9999.0f);
            glm::vec3 v2 = glm::vec3(9999.0f, 0.0f, 9999.0f);

            float distance = 0.0f;
            float u = 0.0f, v = 0.0f;
            bool hit = RayTriangleIntersection(
                rayOrigin,
                rayDirection,
                v0,
                v1,
                v2,
                distance,
                u,
                v
            );

            if (hit && distance < t && distance > 0.0f)
            {
                resultBool = core::K_TRUE;
                t = distance;
            }

            p = rayOrigin + (rayDirection * t);
            result = p;
        }

        bool cRayHit::RayTriangleIntersection(
            const glm::vec3& orig,
            const glm::vec3& dir,
            const glm::vec3& v0,
            const glm::vec3& v1,
            const glm::vec3& v2,
            float& t,
            float& u,
            float& v
        )
        {
            glm::vec3 v0v1 = v1 - v0;
            glm::vec3 v0v2 = v2 - v0;
            glm::vec3 pvec = glm::cross(dir, v0v2);
            float det = glm::dot(v0v1, pvec);
#ifdef CULLING
            // if the determinant is negative, the triangle is 'back facing'
            // if the determinant is close to 0, the ray misses the triangle
            if (det < kEpsilon) return false;
#else
            // ray and triangle are parallel if det is close to 0
            if (fabs(det) < 0.0001f) return false;
#endif
            float invDet = 1.0f / det;

            glm::vec3 tvec = orig - v0;
            u = glm::dot(tvec, pvec) * invDet;
            if (u < 0.0f || u > 1.0f) return false;

            glm::vec3 qvec = glm::cross(tvec, v0v1);
            v = glm::dot(dir, qvec) * invDet;
            if (v < 0.0f || u + v > 1.0f) return false;

            t = glm::dot(v0v2, qvec) * invDet;

            return true;
        }
    }
}