// dynamic_instance_storage.cpp

#include "dynamic_instance_storage.hpp"
#include "game_object.hpp"
#include "game_object_subsystem.hpp"

triton::HRenderInstance triton::XDynamicInstanceStorage::CreateDynamicInstance(const HBatch& batch)
{
	HRenderInstance drih = Create();
	SRenderInstanceData drid = Get(drih);
	drid.batch = batch;

	return drih;
}

void triton::XDynamicInstanceStorage::DestroyDynamicInstance(const HRenderInstance& instance)
{
	Destroy(instance);
}

void triton::XDynamicInstanceStorage::UpdateTransform(const HRenderInstance& instance)
{
	SRenderInstanceData& rid = Get(instance);
	SGameObjectData& god = _context->GetSubsystem<XGameObjectSubsystem>()->Get(rid.gameObject);

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