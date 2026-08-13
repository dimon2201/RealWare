// game_object_subsystem.cpp

#include "game_object_subsystem.hpp"
#include "graphics.hpp"
#include "material_subsystem.hpp"
#include "model3d_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "game_object_pool.hpp"
#include "math.hpp"
#include "model3d_pool.hpp"
#include "geometry_storage.hpp"

using namespace types;

triton::XGameObjectSubsystem::XGameObjectSubsystem(cContext* context) : ISubsys(context)
{
	_pool = CObjectAllocator::Create<XGameObjectPool>(
		64,
		_context,
		K_TRUE
	);
}

triton::XGameObjectSubsystem::~XGameObjectSubsystem()
{
	CObjectAllocator::Destroy<XGameObjectPool>(_pool);
}

std::optional<triton::SGameObjectData::THandle> triton::XGameObjectSubsystem::Create(const std::string& name)
{
	auto handleResult = _pool->Create();
	if (!handleResult.has_value())
		return std::nullopt;
	auto handle = *handleResult;

	auto valueResult = _pool->Get(handle);
	if (!valueResult.has_value())
		return std::nullopt;
	auto value = *valueResult;

	value.get().name = name;

	return handle;
}

void triton::XGameObjectSubsystem::Init()
{
}

void triton::XGameObjectSubsystem::Free()
{
}

void triton::XGameObjectSubsystem::Update()
{
}

std::optional<triton::SStaticRenderInstanceData::THandle> triton::XGameObjectSubsystem::SetRenderableStatic(
	const SGameObjectData::THandle& gameObject,
	const SBatchData::THandle& batch,
	const std::optional<SMaterialData::THandle>& existingMaterial
)
{
	XBatchSubsystem* batchSubsystem = _context->GetSubsystem<XBatchSubsystem>();

	auto valueResult = _pool->Get(gameObject);
	if (!valueResult.has_value())
		return std::nullopt;
	auto value = *valueResult;

	SStaticRenderInstanceData::THandle rih = *batchSubsystem->AddStaticInstance(batch, gameObject);
	
	if (existingMaterial.has_value())
		batchSubsystem->SetStaticInstance(rih, *existingMaterial);

	return rih;
}

std::optional<triton::SDynamicRenderInstanceData::THandle> triton::XGameObjectSubsystem::SetRenderableDynamic(
	const SGameObjectData::THandle& gameObject,
	const SBatchData::THandle& batch,
	const std::optional<SMaterialData::THandle>& existingMaterial
)
{
	XBatchSubsystem* batchSubsystem = _context->GetSubsystem<XBatchSubsystem>();

	auto valueResult = _pool->Get(gameObject);
	if (!valueResult.has_value())
		return std::nullopt;
	auto value = *valueResult;

	SDynamicRenderInstanceData::THandle rih = *batchSubsystem->AddDynamicInstance(batch, gameObject);
	
	if (existingMaterial.has_value())
	{
		auto ri = *_context->GetSubsystem<XBatchSubsystem>()->GetDynamicRenderInstancePool()->Get(rih);
		ri.get().material = *existingMaterial;
	}

	return rih;
}