// gameobject_manager.cpp

#include "application.hpp"
#include "gameobject_manager.hpp"
#include "render_manager.hpp"
#include "physics_manager.hpp"
#include "memory_pool.hpp"

using namespace types;

/*cGameObject::cGameObject(cContext* context) : iObject(context)
{
    sTransform* pTransform = (sTransform*)(memoryPool->Allocate(sizeof(sTransform)));
    _transform = new (pTransform) sTransform();
}

void cGameObject::SetPhysicsActor(eCategory staticOrDynamic, eCategory shapeType, cPhysicsSimulationScene* scene, cPhysicsSubstance* substance, f32 mass)
{
    mPhysics* physics = GetApplication()->GetPhysicsManager();
    _actor = physics->CreateActor(
        GetID(),
        staticOrDynamic,
        shapeType,
        scene,
        substance,
        mass,
        _transform,
        this
    );
}

void cGameObject::SetPhysicsController(f32 eyeHeight, f32 height, f32 radius, const glm::vec3& up, cPhysicsSimulationScene* scene, cPhysicsSubstance* substance)
{
    mPhysics* physics = GetApplication()->GetPhysicsManager();
    _controller = physics->CreateController(
        GetID(),
        eyeHeight,
        height,
        radius,
        _transform,
        up,
        scene,
        substance
    );
}*/