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
#include "animation_subsystem.hpp"

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

std::optional<triton::SGameObjectData::THandle> triton::XGameObjectSubsystem::Create(
	const std::string& name,
	const SBatchData::THandle& batchHandle
)
{
	auto handleResult = _pool->Create();
	if (!handleResult.has_value())
		return std::nullopt;
	auto handle = *handleResult;

	SGameObjectData& gameObject = *_pool->Get(handle);

	SBatchData& batch = *_context->GetSubsystem<XBatchSubsystem>()->GetBatchPool()->Get(batchHandle);
	if (batch.motionType == ERenderInstanceMotionType::Static)
	{
		gameObject.motionType = ERenderInstanceMotionType::Static;
		SetRenderableStatic(handle, batchHandle);
		_context->GetSubsystem<XBatchSubsystem>()->SetStaticInstanceWorldMatrix(gameObject.staticRenderInstance, cMatrix4(1.0f));
	}
	else if (batch.motionType == ERenderInstanceMotionType::Dynamic)
	{
		gameObject.motionType = ERenderInstanceMotionType::Dynamic;
		SetRenderableDynamic(handle, batchHandle);
		_context->GetSubsystem<XBatchSubsystem>()->SetDynamicInstanceWorldMatrix(gameObject.dynamicRenderInstance, cMatrix4(1.0f));
	}

	return handle;
}

std::optional<triton::SGameObjectData::THandle> triton::XGameObjectSubsystem::Create(
	const std::string& name,
	const SModel3DData& model,
	const SBatchData::THandle& batchHandle
)
{
	auto handle = *Create(name, batchHandle);

	SGameObjectData& gameObject = *_pool->Get(handle);

	gameObject.skeleton = model.skeleton;
	gameObject.skin = *_context->GetSubsystem<XSkinningSubsystem>()->Create(gameObject.skeleton);
	if (gameObject.motionType == ERenderInstanceMotionType::Static)
	{
		_context->GetSubsystem<XBatchSubsystem>()->SetStaticInstanceSkin(gameObject.staticRenderInstance, gameObject.skin);
	}
	else if (gameObject.motionType == ERenderInstanceMotionType::Dynamic)
	{
	}
	gameObject.animations = model.animations;

	return handle;
}

void triton::XGameObjectSubsystem::Destroy(const SGameObjectData::THandle& gameObject)
{
	_pool->Destroy(gameObject);
}

std::optional<triton::SStaticRenderInstanceData::THandle> triton::XGameObjectSubsystem::SetRenderableStatic(
	const SGameObjectData::THandle& gameObject,
	const SBatchData::THandle& batch
)
{
	SGameObjectData& data = *_pool->Get(gameObject);

	auto handleResult = _context->GetSubsystem<XBatchSubsystem>()->AddStaticInstance(batch);
	if (!handleResult.has_value())
		return std::nullopt;

	data.staticRenderInstance = *handleResult;

	return *handleResult;
}

std::optional<triton::SDynamicRenderInstanceData::THandle> triton::XGameObjectSubsystem::SetRenderableDynamic(
	const SGameObjectData::THandle& gameObject,
	const SBatchData::THandle& batch
)
{
	SGameObjectData& data = *_pool->Get(gameObject);

	auto handleResult = _context->GetSubsystem<XBatchSubsystem>()->AddDynamicInstance(batch);
	if (!handleResult.has_value())
		return std::nullopt;

	data.dynamicRenderInstance = *handleResult;

	return *handleResult;
}

void triton::XGameObjectSubsystem::RemoveRenderableStatic(const SGameObjectData::THandle& gameObject)
{
	SGameObjectData& data = *_pool->Get(gameObject);

	_context->GetSubsystem<XBatchSubsystem>()->RemoveStaticInstance(data.batch, data.staticRenderInstance);
}

void triton::XGameObjectSubsystem::RemoveRenderableDynamic(const SGameObjectData::THandle& gameObject)
{
	SGameObjectData& data = *_pool->Get(gameObject);

	_context->GetSubsystem<XBatchSubsystem>()->RemoveDynamicInstance(data.batch, data.dynamicRenderInstance);
}

void triton::XGameObjectSubsystem::PlayAnimation(const SGameObjectData::THandle& gameObject, types::usize index)
{
	SGameObjectData& data = *_pool->Get(gameObject);

	static f32 time = 0.0f;
	SFrame frame = *_context->GetSubsystem<XAnimationSubsystem>()->Evaluate(
		data.skeleton,
		data.animations[index],
		time
	);
	_context->GetSubsystem<XSkinningSubsystem>()->Skin(
		data.skin,
		frame
	);
	time += 0.1f;
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