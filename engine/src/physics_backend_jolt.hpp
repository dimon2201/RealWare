// physics_backend_jolt.hpp

#pragma once

#include "physics_backend.hpp"

namespace triton
{
    class XPhysicsBackendJolt final : public IPhysicsBackend
    {
        TRITON_OBJECT(XPhysicsBackendJolt)

    public:
        explicit XPhysicsBackendJolt(cContext* context);
        ~XPhysicsBackendJolt() override;

        SPhysicsWorldBackendData CreateWorld() override final;
        void DestroyWorld(const SPhysicsWorldBackendData& world) override final;

        SPhysicsShapeBackendData CreateBoxShape(const SPhysicsShapeDesc& desc) override final;
        SPhysicsShapeBackendData CreateSphereShape(const SPhysicsShapeDesc& desc) override final;
        SPhysicsShapeBackendData CreateCapsuleShape(const SPhysicsShapeDesc& desc) override final;
        SPhysicsShapeBackendData CreateCylinderShape(const SPhysicsShapeDesc& desc) override final;
        SPhysicsShapeBackendData CreateConvexHullShape(const SPhysicsShapeDesc& desc) override final;
        SPhysicsShapeBackendData CreateTriangleMeshShape(const SPhysicsShapeDesc& desc) override final;
        SPhysicsShapeBackendData CreateHeightFieldShape(const SPhysicsShapeDesc& desc) override final;
        void DestroyShape(const SPhysicsShapeBackendData& shape) override final;

        SPhysicsRigidBodyBackendData CreateRigidBody(
            const SPhysicsRigidBodyDesc& desc,
            const SPhysicsShapeBackendData& shape
        ) override final;
        void DestroyRigidBody(const SPhysicsRigidBodyBackendData& body) override final;

        SPhysicsCharacterControllerBackendData CreateCharacterController(const SPhysicsCharacterControllerDesc& desc) override final;
        void DestroyCharacterController(const SPhysicsCharacterControllerBackendData& controller) override final;

        SPhysicsConstraintBackendData CreateFixedConstraint(const SPhysicsConstraintDesc& desc) override final;
        SPhysicsConstraintBackendData CreateHingeConstraint(const SPhysicsConstraintDesc& desc) override final;
        SPhysicsConstraintBackendData CreateSliderConstraint(const SPhysicsConstraintDesc& desc) override final;
        SPhysicsConstraintBackendData CreateDistanceConstraint(const SPhysicsConstraintDesc& desc) override final;
        SPhysicsConstraintBackendData CreateConeConstraint(const SPhysicsConstraintDesc& desc) override final;
        SPhysicsConstraintBackendData CreateSwingTwistConstraint(const SPhysicsConstraintDesc& desc) override final;
        void DestroyConstraint(const SPhysicsConstraintBackendData& constraint) override final;

        SPhysicsMaterialBackendData CreateMaterial(const SPhysicsMaterialDesc& desc) override final;
        void DestroyMaterial(const SPhysicsMaterialBackendData& material) override final;
    };
}