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
	EVertexBufferFormat format,
	const types::u8* vertexBytes,
	types::usize vertexBytesCount,
	const types::u8* indexBytes,
	types::usize indexBytesCount,
	const std::optional<SBatchData::THandle>& existingBatch,
	const std::optional<SMaterialData::THandle>& existingMaterial
)
{
	XBatchSubsystem* batchSubsystem = _context->GetSubsystem<XBatchSubsystem>();

	auto valueResult = _pool->Get(gameObject);
	if (!valueResult.has_value())
		return std::nullopt;
	auto value = *valueResult;

	SBatchData::THandle batch;
	if (!existingBatch.has_value())
	{
		SGeometryView geometry = *_context->GetSubsystem<XGeometryStorage>()->Create(
			format,
			vertexBytes,
			vertexBytesCount,
			indexBytes,
			indexBytesCount
		);
		batch = *batchSubsystem->Create(format, ERenderInstanceMotionType::Static, geometry);
	}
	else
	{
		batch = *existingBatch;
	}
	
	SStaticRenderInstanceData::THandle rih = *batchSubsystem->AddStaticInstance(batch, gameObject);

	if (existingMaterial.has_value())
	{
		auto ri = *batchSubsystem->GetStaticRenderInstancePool()->Get(rih);
		ri.get().material = *existingMaterial;
	}

	return rih;
}

std::optional<triton::SStaticRenderInstanceData::THandle> triton::XGameObjectSubsystem::SetRenderableStatic(
	const SGameObjectData::THandle& gameObject,
	const SModel3DData::THandle& model,
	const std::optional<SBatchData::THandle>& existingBatch,
	const std::optional<SMaterialData::THandle>& existingMaterial
)
{
	auto valueResult = _pool->Get(gameObject);
	if (!valueResult.has_value())
		return std::nullopt;
	auto value = *valueResult;

	SModel3DData& m3dd = *_context->GetSubsystem<XModel3DSubsystem>()->GetPool()->Get(model);
	
	SBatchData::THandle batch;
	if (!existingBatch.has_value())
	{
		// TODO: Use proper vertex buffer format here
		// ||||||||||||||||||||||||||||||||||||||||||
		// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
		SGeometryView geometry = *_context->GetSubsystem<XGeometryStorage>()->Create(
			EVertexBufferFormat::Skinned_84,
			(u8*)m3dd.vertexData,
			m3dd.vertexCount * sizeof(SStaticVertexGPULayout),
			(u8*)m3dd.indexData,
			m3dd.indexCount * sizeof(u32)
		);
		batch = *_context->GetSubsystem<XBatchSubsystem>()->Create(EVertexBufferFormat::Skinned_84, ERenderInstanceMotionType::Static, geometry);
	}
	else
	{
		batch = *existingBatch;
	}

	SStaticRenderInstanceData::THandle rih = *_context->GetSubsystem<XBatchSubsystem>()->AddStaticInstance(batch, gameObject);

	if (existingMaterial.has_value())
	{
		auto ri = *_context->GetSubsystem<XBatchSubsystem>()->GetStaticRenderInstancePool()->Get(rih);
		ri.get().material = *existingMaterial;
	}
	else
	{
		auto ri = *_context->GetSubsystem<XBatchSubsystem>()->GetStaticRenderInstancePool()->Get(rih);
		ri.get().material.Invalidate();
	}

	return rih;
}

std::optional<triton::SDynamicRenderInstanceData::THandle> triton::XGameObjectSubsystem::SetRenderableDynamic(
	const SGameObjectData::THandle& gameObject,
	EVertexBufferFormat format,
	const types::u8* vertexBytes,
	types::usize vertexBytesCount,
	const types::u8* indexBytes,
	types::usize indexBytesCount,
	const std::optional<SBatchData::THandle>& existingBatch,
	const std::optional<SMaterialData::THandle>& existingMaterial
)
{
	XBatchSubsystem* batchSubsystem = _context->GetSubsystem<XBatchSubsystem>();

	auto valueResult = _pool->Get(gameObject);
	if (!valueResult.has_value())
		return std::nullopt;
	auto value = *valueResult;

	SBatchData::THandle batch;
	if (!existingBatch.has_value())
	{
		SGeometryView geometry = *_context->GetSubsystem<XGeometryStorage>()->Create(
			format,
			vertexBytes,
			vertexBytesCount,
			indexBytes,
			indexBytesCount
		);
		batch = *batchSubsystem->Create(format, ERenderInstanceMotionType::Dynamic, geometry);
	}
	else
	{
		batch = *existingBatch;
	}

	SDynamicRenderInstanceData::THandle rih = *batchSubsystem->AddDynamicInstance(batch, gameObject);

	if (existingMaterial.has_value())
	{
		auto ri = *_context->GetSubsystem<XBatchSubsystem>()->GetDynamicRenderInstancePool()->Get(rih);
		ri.get().material = *existingMaterial;
	}

	return rih;
}

std::optional<triton::SDynamicRenderInstanceData::THandle> triton::XGameObjectSubsystem::SetRenderableDynamic(
	const SGameObjectData::THandle& gameObject,
	const SModel3DData::THandle& model,
	const std::optional<SBatchData::THandle>& existingBatch,
	const std::optional<SMaterialData::THandle>& existingMaterial
)
{
	auto valueResult = _pool->Get(gameObject);
	if (!valueResult.has_value())
		return std::nullopt;
	auto value = *valueResult;

	SModel3DData& m3dd = *_context->GetSubsystem<XModel3DSubsystem>()->GetPool()->Get(model);

	SBatchData::THandle batch;
	if (!existingBatch.has_value())
	{
		// TODO: Use proper vertex buffer format here
		// ||||||||||||||||||||||||||||||||||||||||||
		// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
		SGeometryView geometry = *_context->GetSubsystem<XGeometryStorage>()->Create(
			EVertexBufferFormat::Skinned_84,
			(u8*)m3dd.vertexData,
			m3dd.vertexCount * sizeof(SSkinnedVertexGPULayout),
			(u8*)m3dd.indexData,
			m3dd.indexCount * sizeof(u32)
		);
		batch = *_context->GetSubsystem<XBatchSubsystem>()->Create(EVertexBufferFormat::Skinned_84, ERenderInstanceMotionType::Dynamic, geometry);
	}
	else
	{
		batch = *existingBatch;
	}

	SDynamicRenderInstanceData::THandle rih = *_context->GetSubsystem<XBatchSubsystem>()->AddDynamicInstance(batch, gameObject);

	auto ri = *_context->GetSubsystem<XBatchSubsystem>()->GetDynamicRenderInstancePool()->Get(rih);
	ri.get().skeleton = m3dd.skeleton;

	if (existingMaterial.has_value())
	{
		ri.get().material = *existingMaterial;
	}
	else
	{
		ri.get().material.Invalidate();
	}

	return rih;
}