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

	return *handleResult;
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
	boolean bIsRenderable,
	const SBatchData::THandle& batch
)
{
	SGameObjectData& data = *_pool->Get(gameObject);

	if (bIsRenderable == True)
	{
		if (_context->GetSubsystem<XBatchSubsystem>()->
			GetStaticRenderInstancePool()->
			Get(data.staticRenderInstance).has_value())
			return std::nullopt;

		auto handleResult = _context->GetSubsystem<XBatchSubsystem>()->AddStaticInstance(batch);
		if (!handleResult.has_value())
			return std::nullopt;

		data.staticRenderInstance = *handleResult;

		return *handleResult;
	}
	else if (bIsRenderable == False)
	{
		if (!_context->GetSubsystem<XBatchSubsystem>()->
			GetStaticRenderInstancePool()->
			Get(data.staticRenderInstance).has_value())
			return std::nullopt;

		_context->GetSubsystem<XBatchSubsystem>()->RemoveStaticInstance(data.batch, data.staticRenderInstance);
	}
}

std::optional<triton::SDynamicRenderInstanceData::THandle> triton::XGameObjectSubsystem::SetRenderableDynamic(
	const SGameObjectData::THandle& gameObject,
	boolean bIsRenderable,
	const SBatchData::THandle& batch
)
{
	SGameObjectData& data = *_pool->Get(gameObject);

	if (bIsRenderable == True)
	{
		if (_context->GetSubsystem<XBatchSubsystem>()->
			GetDynamicRenderInstancePool()->
			Get(data.dynamicRenderInstance).has_value())
			return std::nullopt;

		auto handleResult = _context->GetSubsystem<XBatchSubsystem>()->AddDynamicInstance(batch);
		if (!handleResult.has_value())
			return std::nullopt;

		data.dynamicRenderInstance = *handleResult;

		return *handleResult;
	}
	else if (bIsRenderable == False)
	{
		if (!_context->GetSubsystem<XBatchSubsystem>()->
			GetDynamicRenderInstancePool()->
			Get(data.dynamicRenderInstance).has_value())
			return std::nullopt;

		_context->GetSubsystem<XBatchSubsystem>()->RemoveDynamicInstance(data.batch, data.dynamicRenderInstance);

		return std::nullopt;
	}
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