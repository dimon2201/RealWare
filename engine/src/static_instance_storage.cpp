// static_instance_storage.cpp

#include "static_instance_storage.hpp"
#include "game_object.hpp"
#include "game_object_subsystem.hpp"

triton::HRenderInstance triton::XStaticInstanceStorage::CreateStaticInstance(const HBatch& batch)
{
	HRenderInstance srih = Create();
	SRenderInstanceData& srid = Get(srih);
	srid.batch = batch;

	return srih;
}

void triton::XStaticInstanceStorage::DestroyStaticInstance(const HRenderInstance& instance)
{
	Destroy(instance);
}

void triton::XStaticInstanceStorage::UpdateTransform(const HRenderInstance& instance)
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