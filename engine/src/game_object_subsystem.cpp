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

std::optional<triton::XGameObject::THandle> triton::XGameObjectSubsystem::Create(
	const std::string& name,
	const SBatchData::THandle& batchHandle
)
{
	auto handleResult = _pool->Create(_context, batchHandle);
	if (!handleResult.has_value())
		return std::nullopt;
	auto handle = *handleResult;

	return handle;
}

std::optional<triton::XGameObject::THandle> triton::XGameObjectSubsystem::Create(
	const std::string& name,
	const SModel3DData& model,
	const SBatchData::THandle& batchHandle
)
{
	auto handleResult = _pool->Create(_context, model, batchHandle);
	if (!handleResult.has_value())
		return std::nullopt;
	auto handle = *handleResult;

	return handle;
}

void triton::XGameObjectSubsystem::Destroy(const XGameObject::THandle& gameObject)
{
	_pool->Destroy(gameObject);
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