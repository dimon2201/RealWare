// static_instance_storage.cpp

#include "static_instance_storage.hpp"
#include "game_object.hpp"
#include "game_object_subsystem.hpp"

triton::HRenderInstance triton::CStaticInstanceStorage::CreateStaticInstance(const HBatch& batch)
{
	HRenderInstance srih = Create();
	Get(srih).batch = batch;
	
	return srih;
}

void triton::CStaticInstanceStorage::DestroyStaticInstance(const HRenderInstance& instance)
{
	Destroy(instance);
}

void triton::CStaticInstanceStorage::UpdateTransform(const HRenderInstance& instance, XGameObjectSubsystem* gameObjectSubsystem)
{
	SRenderInstanceData& rid = Get(instance);
	SGameObjectData& god = gameObjectSubsystem->Get(rid.gameObject);
	
	rid.worldMatrix = cMath::Transform(
		god.worldPosition,
		god.worldRotation,
		god.scale
	);

	WriteFieldToStaging<cMatrix4>(
		instance,
		16,
		rid.worldMatrix
	);
}