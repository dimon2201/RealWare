// physics_backend.hpp

#pragma once

#include <unordered_map>
#include <string>
#include "backend.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    struct SPair
    {
        SPair() = default;
        SPair(const std::string& tag, void* ptr) : tag(tag), ptr(ptr) {}

        std::string tag = {};
        void* ptr = nullptr;
    };

    struct SPhysicsWorldDesc final
    {
    };

    static constexpr types::usize kMaxPhysicsWorldSystemObjectCount = 16;

    struct SPhysicsWorldBackendData final
    {
        types::usize worldIndex = 0;
        types::usize systemObjectCount = 0;
        SPair systemObjects[kMaxPhysicsWorldSystemObjectCount];
    };

    struct SPhysicsShapeDesc final
    {
        cVector3 halfExtent = cVector3(1.0f);
    };

    struct SPhysicsShapeBackendData final
    {
        void* shape = nullptr;
    };

    enum class EMotionType
    {
        Static,
        Kinematic,
        Dynamic
    };

    static constexpr types::usize kMaxPhysicsRigidBodyPropertyCount = 4;

    struct SPhysicsRigidBodyDesc final
    {
        cVector3 worldPosition = cVector3(0.0f);
        cQuaternion worldRotation = cQuaternion();
        EMotionType motionType = EMotionType::Static;
        types::qword properties[kMaxPhysicsRigidBodyPropertyCount] = {};
    };

    struct SPhysicsRigidBodyBackendData final
    {
        void* body = nullptr;
    };

    struct SPhysicsCharacterControllerDesc final
    {
    };

    struct SPhysicsCharacterControllerBackendData final
    {
    };

    struct SPhysicsConstraintDesc final
    {
    };

    struct SPhysicsConstraintBackendData final
    {
    };

    struct SPhysicsMaterialDesc final
    {
    };

    struct SPhysicsMaterialBackendData final
    {
    };

    class IPhysicsBackend : public iBackend
    {
        TRITON_OBJECT(IPhysicsBackend)

    public:
        explicit IPhysicsBackend(cContext* context) : iBackend(context) {}
        ~IPhysicsBackend() override = default;

        virtual SPhysicsWorldBackendData CreateWorld() = 0;

        virtual void DestroyWorld(const SPhysicsWorldBackendData& world) = 0;

        virtual SPhysicsShapeBackendData CreateBoxShape(
            const SPhysicsWorldBackendData& world,
            const SPhysicsShapeDesc& desc
        ) = 0;

        virtual SPhysicsShapeBackendData CreateSphereShape(const SPhysicsShapeDesc& desc) = 0;
        virtual SPhysicsShapeBackendData CreateCapsuleShape(const SPhysicsShapeDesc& desc) = 0;
        virtual SPhysicsShapeBackendData CreateCylinderShape(const SPhysicsShapeDesc& desc) = 0;
        virtual SPhysicsShapeBackendData CreateConvexHullShape(const SPhysicsShapeDesc& desc) = 0;
        virtual SPhysicsShapeBackendData CreateTriangleMeshShape(const SPhysicsShapeDesc& desc) = 0;
        virtual SPhysicsShapeBackendData CreateHeightFieldShape(const SPhysicsShapeDesc& desc) = 0;
        virtual void DestroyShape(const SPhysicsShapeBackendData& shape) = 0;

        virtual SPhysicsRigidBodyBackendData CreateRigidBody(
            const SPhysicsWorldBackendData& world,
            const SPhysicsRigidBodyDesc& desc,
            const SPhysicsShapeBackendData& shape
        ) = 0;
        virtual void DestroyRigidBody(
            const SPhysicsWorldBackendData& world,
            const SPhysicsRigidBodyBackendData& body
        ) = 0;

        virtual SPhysicsCharacterControllerBackendData CreateCharacterController(const SPhysicsCharacterControllerDesc& desc) = 0;
        virtual void DestroyCharacterController(const SPhysicsCharacterControllerBackendData& controller) = 0;

        virtual SPhysicsConstraintBackendData CreateFixedConstraint(const SPhysicsConstraintDesc& desc) = 0;
        virtual SPhysicsConstraintBackendData CreateHingeConstraint(const SPhysicsConstraintDesc& desc) = 0;
        virtual SPhysicsConstraintBackendData CreateSliderConstraint(const SPhysicsConstraintDesc& desc) = 0;
        virtual SPhysicsConstraintBackendData CreateDistanceConstraint(const SPhysicsConstraintDesc& desc) = 0;
        virtual SPhysicsConstraintBackendData CreateConeConstraint(const SPhysicsConstraintDesc& desc) = 0;
        virtual SPhysicsConstraintBackendData CreateSwingTwistConstraint(const SPhysicsConstraintDesc& desc) = 0;
        virtual void DestroyConstraint(const SPhysicsConstraintBackendData& constraint) = 0;

        virtual SPhysicsMaterialBackendData CreateMaterial(const SPhysicsMaterialDesc& desc) = 0;
        virtual void DestroyMaterial(const SPhysicsMaterialBackendData& material) = 0;

        virtual void OptimizeAccelerationStructures(const SPhysicsWorldBackendData& world) = 0;

        virtual void Update(
            const SPhysicsWorldBackendData& world,
            types::f32 deltaTime,
            types::s32 collisionStepCount
        ) = 0;
    };
}