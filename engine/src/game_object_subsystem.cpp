// game_object_subsystem.cpp

#include "game_object_subsystem.hpp"
#include "graphics.hpp"
#include "material_subsystem.hpp"
#include "model3d_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "math.hpp"
#include "static_instance_storage.hpp"

using namespace types;

void triton::XGameObjectSubsystem::Init()
{
}

void triton::XGameObjectSubsystem::Free()
{
}

void triton::XGameObjectSubsystem::Update()
{
}

triton::HRenderInstance triton::XGameObjectSubsystem::SetRenderable(
	const HGameObject& gameObject,
	ERenderInstanceMotionType motionType,
	EGraphicsBufferFormat format,
	const types::u8* vertexBytes,
	types::usize vertexBytesCount,
	const types::u8* indexBytes,
	types::usize indexBytesCount,
	const std::optional<HBatch>& existingBatch,
	const std::optional<HMaterial>& existingMaterial
)
{
	SGameObjectData& go = _objects->Get(gameObject);
	if (go.motionType == ERenderInstanceMotionType::Static)
	{
		if (_context->GetSubsystem<XBatchSubsystem>()->GetStaticInstanceStorage().Exists(go.staticRenderInstance))
			return {};
	}
	else if (go.motionType == ERenderInstanceMotionType::Dynamic)
	{
		if (_context->GetSubsystem<XBatchSubsystem>()->GetDynamicInstanceStorage().Exists(go.dynamicRenderInstance))
			return {};
	}

	HBatch batch;
	if (!existingBatch.has_value())
	{
		SGeometryView geometry = *_context->GetSubsystem<cGraphics>()->StoreGeometry(
			format,
			vertexBytes,
			vertexBytesCount,
			indexBytes,
			indexBytesCount
		);
		batch = *_context->GetSubsystem<XBatchSubsystem>()->Create(motionType, geometry);
	}
	else
	{
		batch = *existingBatch;
	}
	
	HRenderInstance ri;

	if (motionType == ERenderInstanceMotionType::Static)
		ri = _context->GetSubsystem<XBatchSubsystem>()->AddStaticInstance(batch, gameObject);
	else if (motionType == ERenderInstanceMotionType::Dynamic)
		ri = _context->GetSubsystem<XBatchSubsystem>()->AddDynamicInstance(batch, gameObject);
	else
		return HRenderInstance();

	if (existingMaterial.has_value())
	{
		if (motionType == ERenderInstanceMotionType::Static)
			_context->GetSubsystem<XBatchSubsystem>()->GetStaticInstanceStorage().Get(ri).material = *existingMaterial;
		else if (motionType == ERenderInstanceMotionType::Dynamic)
			_context->GetSubsystem<XBatchSubsystem>()->GetDynamicInstanceStorage().Get(ri).material = *existingMaterial;
	}

	return ri;
}

triton::HRenderInstance triton::XGameObjectSubsystem::SetRenderable(
	const HGameObject& gameObject,
	ERenderInstanceMotionType motionType,
	const HModel3D& model,
	const std::optional<HBatch>& existingBatch,
	const std::optional<HMaterial>& existingMaterial
)
{
	SGameObjectData& go = Get(gameObject);
	if (go.motionType == ERenderInstanceMotionType::Static)
	{
		if (_context->GetSubsystem<XBatchSubsystem>()->GetStaticInstanceStorage().Exists(go.staticRenderInstance))
			return {};
	}
	else if (go.motionType == ERenderInstanceMotionType::Dynamic)
	{
		if (_context->GetSubsystem<XBatchSubsystem>()->GetDynamicInstanceStorage().Exists(go.dynamicRenderInstance))
			return {};
	}

	SModel3DData& m3dd = _context->GetSubsystem<XModel3DSubsystem>()->Get(model);
	
	HBatch batch;
	if (!existingBatch.has_value())
	{
		SGeometryView geometry = *_context->GetSubsystem<cGraphics>()->StoreGeometry(
			EGraphicsBufferFormat::POSITION_TEXCOORD_NORMAL_TANGENT_VEC3_VEC2_VEC3_VEC4,
			(u8*)m3dd.vertexData,
			m3dd.vertexCount * sizeof(SVertex),
			(u8*)m3dd.indexData,
			m3dd.indexCount * sizeof(u32)
		);
		batch = *_context->GetSubsystem<XBatchSubsystem>()->Create(motionType, geometry);
	}
	else
	{
		batch = *existingBatch;
	}

	HRenderInstance ri;
	
	if (motionType == ERenderInstanceMotionType::Static)
		ri = _context->GetSubsystem<XBatchSubsystem>()->AddStaticInstance(batch, gameObject);
	else if (motionType == ERenderInstanceMotionType::Dynamic)
		ri = _context->GetSubsystem<XBatchSubsystem>()->AddDynamicInstance(batch, gameObject);
	else
		return HRenderInstance();

	if (motionType == ERenderInstanceMotionType::Static)
		_context->GetSubsystem<XBatchSubsystem>()->GetStaticInstanceStorage().Get(ri).skeleton = m3dd.skeleton;
	else if (motionType == ERenderInstanceMotionType::Dynamic)
		_context->GetSubsystem<XBatchSubsystem>()->GetDynamicInstanceStorage().Get(ri).material = m3dd.skeleton;

	if (existingMaterial.has_value())
	{
		if (motionType == ERenderInstanceMotionType::Static)
			_context->GetSubsystem<XBatchSubsystem>()->GetStaticInstanceStorage().Get(ri).material = *existingMaterial;
		else if (motionType == ERenderInstanceMotionType::Dynamic)
			_context->GetSubsystem<XBatchSubsystem>()->GetDynamicInstanceStorage().Get(ri).material = *existingMaterial;
	}
	else
	{
		if (motionType == ERenderInstanceMotionType::Static)
			_context->GetSubsystem<XBatchSubsystem>()->GetStaticInstanceStorage().Get(ri).material.Invalidate();
		else if (motionType == ERenderInstanceMotionType::Dynamic)
			_context->GetSubsystem<XBatchSubsystem>()->GetDynamicInstanceStorage().Get(ri).material.Invalidate();
	}

	return ri;
}