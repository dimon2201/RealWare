// dynamic_instance_storage.cpp

#include "dynamic_instance_storage.hpp"
#include "game_object.hpp"
#include "game_object_subsystem.hpp"

triton::HRenderInstance triton::CDynamicInstanceStorage::CreateDynamicInstance(const HBatch& batch)
{
	HRenderInstance drih = Create();
	SRenderInstanceData drid = Get(drih);
	drid.batch = batch;

	return drih;
}

void triton::CDynamicInstanceStorage::DestroyDynamicInstance(const HRenderInstance& instance)
{
	Destroy(instance);
}

void triton::CDynamicInstanceStorage::UpdateTransform(const HRenderInstance& instance, XGameObjectSubsystem* gameObjectSubsystem)
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