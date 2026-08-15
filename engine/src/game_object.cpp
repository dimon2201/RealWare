// game_object.cpp

#include "game_object.hpp"
#include "context.hpp"
#include "batcher.hpp"
#include "animation_subsystem.hpp"

using namespace types;

triton::XGameObject::XGameObject(cContext* context, const SBatchData::THandle& batchHandle) : iObject(context)
{
	SBatchData& batch = *_context->GetSubsystem<XBatchSubsystem>()->GetBatchPool()->Get(batchHandle);
	if (batch.motionType == ERenderInstanceMotionType::Static)
	{
		_motionType = ERenderInstanceMotionType::Static;
		SetRenderableStatic(batchHandle);
		_context->GetSubsystem<XBatchSubsystem>()->SetStaticInstanceWorldMatrix(_staticRenderInstance, cMatrix4(1.0f));
	}
	else if (batch.motionType == ERenderInstanceMotionType::Dynamic)
	{
		_motionType = ERenderInstanceMotionType::Dynamic;
		SetRenderableDynamic(batchHandle);
		_context->GetSubsystem<XBatchSubsystem>()->SetDynamicInstanceWorldMatrix(_dynamicRenderInstance, cMatrix4(1.0f));
	}
}

triton::XGameObject::XGameObject(
	cContext* context,
	const SModel3DData& model,
	const SBatchData::THandle& batchHandle
) : XGameObject(context, batchHandle)
{
	_skeleton = model.skeleton;
	_skin = *_context->GetSubsystem<XSkinningSubsystem>()->Create(_skeleton);
	if (_motionType == ERenderInstanceMotionType::Static)
	{
		_context->GetSubsystem<XBatchSubsystem>()->SetStaticInstanceSkin(_staticRenderInstance, _skin);
	}
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
	{
	}
	_animations = model.animations;
}

triton::XGameObject::~XGameObject()
{
	if (_motionType == ERenderInstanceMotionType::Static)
		RemoveRenderableStatic();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		RemoveRenderableDynamic();
}

std::optional<triton::SStaticRenderInstanceData::THandle> triton::XGameObject::SetRenderableStatic(
	const SBatchData::THandle& batch
)
{
	if (_staticRenderInstance.IsInvalid() == K_FALSE)
		return std::nullopt;

	auto handleResult = _context->GetSubsystem<XBatchSubsystem>()->AddStaticInstance(batch);
	if (!handleResult.has_value())
		return std::nullopt;

	_staticRenderInstance = *handleResult;

	return *handleResult;
}

std::optional<triton::SDynamicRenderInstanceData::THandle> triton::XGameObject::SetRenderableDynamic(
	const SBatchData::THandle& batch
)
{
	if (_dynamicRenderInstance.IsInvalid() == K_FALSE)
		return std::nullopt;

	auto handleResult = _context->GetSubsystem<XBatchSubsystem>()->AddDynamicInstance(batch);
	if (!handleResult.has_value())
		return std::nullopt;

	_dynamicRenderInstance = *handleResult;

	return *handleResult;
}

void triton::XGameObject::RemoveRenderableStatic()
{
	if (_staticRenderInstance.IsInvalid() == K_FALSE)
	{
		_context->GetSubsystem<XBatchSubsystem>()->RemoveStaticInstance(_batch, _staticRenderInstance);
		_staticRenderInstance.Invalidate();
	}
}

void triton::XGameObject::RemoveRenderableDynamic()
{
	if (_dynamicRenderInstance.IsInvalid() == K_FALSE)
	{
		_context->GetSubsystem<XBatchSubsystem>()->RemoveDynamicInstance(_batch, _dynamicRenderInstance);
		_dynamicRenderInstance.Invalidate();
	}
}

void triton::XGameObject::PlayAnimation(types::usize index)
{
	static f32 time = 0.0f;
	SFrame frame = *_context->GetSubsystem<XAnimationSubsystem>()->Evaluate(
		_skeleton,
		_animations[index],
		time
	);
	_context->GetSubsystem<XSkinningSubsystem>()->Skin(
		_skin,
		frame
	);
	time += 0.1f;
}