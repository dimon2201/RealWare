// game_object_subsystem.cpp

#include "game_object_subsystem.hpp"
#include "graphics.hpp"
#include "material_subsystem.hpp"
#include "model3d_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "math.hpp"

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

triton::HStaticRenderInstance triton::XGameObjectSubsystem::SetRenderableStatic(
	const HGameObject& gameObject,
	EGraphicsBufferFormat format,
	const types::u8* vertexBytes,
	types::usize vertexBytesCount,
	const types::u8* indexBytes,
	types::usize indexBytesCount,
	const std::optional<HBatch>& existingBatch,
	const std::optional<HMaterial>& existingMaterial
)
{
	XBatchSubsystem* batchSubsystem = _context->GetSubsystem<XBatchSubsystem>();

	SGameObjectData& go = _objects->Get(gameObject);
	if (batchSubsystem->CUploader<
		SStaticRenderInstanceData,
		HStaticRenderInstance,
		XLinearArray<SStaticRenderInstanceData>,
		SGPUStaticRenderInstanceLayout
	>::Exists(go.staticRenderInstance))
		return {};

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
		batch = *batchSubsystem->Create(ERenderInstanceMotionType::Static, geometry);
	}
	else
	{
		batch = *existingBatch;
	}
	
	HStaticRenderInstance ri;

	ri = batchSubsystem->AddStaticInstance(batch, gameObject);

	if (existingMaterial.has_value())
	{
		batchSubsystem->CUploader<
			SStaticRenderInstanceData,
			HStaticRenderInstance,
			XLinearArray<SStaticRenderInstanceData>,
			SGPUStaticRenderInstanceLayout
		>::Get(ri).material = *existingMaterial;
	}

	return ri;
}

triton::HStaticRenderInstance triton::XGameObjectSubsystem::SetRenderableStatic(
	const HGameObject& gameObject,
	const HModel3D& model,
	const std::optional<HBatch>& existingBatch,
	const std::optional<HMaterial>& existingMaterial
)
{
	SGameObjectData& go = Get(gameObject);
	if (_context->GetSubsystem<XBatchSubsystem>()->CUploader<
		SStaticRenderInstanceData,
		HStaticRenderInstance,
		XLinearArray<SStaticRenderInstanceData>,
		SGPUStaticRenderInstanceLayout
	>::Exists(go.staticRenderInstance))
		return {};

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
		batch = *_context->GetSubsystem<XBatchSubsystem>()->Create(ERenderInstanceMotionType::Static, geometry);
	}
	else
	{
		batch = *existingBatch;
	}

	HStaticRenderInstance ri;
	
	ri = _context->GetSubsystem<XBatchSubsystem>()->AddStaticInstance(batch, gameObject);

	_context->GetSubsystem<XBatchSubsystem>()->CUploader<
		SStaticRenderInstanceData,
		HStaticRenderInstance,
		XLinearArray<SStaticRenderInstanceData>,
		SGPUStaticRenderInstanceLayout
	>::Get(ri).skeleton = m3dd.skeleton;

	if (existingMaterial.has_value())
	{
		_context->GetSubsystem<XBatchSubsystem>()->CUploader<
			SStaticRenderInstanceData,
			HStaticRenderInstance,
			XLinearArray<SStaticRenderInstanceData>,
			SGPUStaticRenderInstanceLayout
		>::Get(ri).material = *existingMaterial;
	}
	else
	{
		_context->GetSubsystem<XBatchSubsystem>()->CUploader<
			SStaticRenderInstanceData,
			HStaticRenderInstance,
			XLinearArray<SStaticRenderInstanceData>,
			SGPUStaticRenderInstanceLayout
		>::Get(ri).material.Invalidate();
	}

	return ri;
}

triton::HDynamicRenderInstance triton::XGameObjectSubsystem::SetRenderableDynamic(
	const HGameObject& gameObject,
	EGraphicsBufferFormat format,
	const types::u8* vertexBytes,
	types::usize vertexBytesCount,
	const types::u8* indexBytes,
	types::usize indexBytesCount,
	const std::optional<HBatch>& existingBatch,
	const std::optional<HMaterial>& existingMaterial
)
{
	XBatchSubsystem* batchSubsystem = _context->GetSubsystem<XBatchSubsystem>();

	SGameObjectData& go = _objects->Get(gameObject);
	if (batchSubsystem->CUploader<
		SDynamicRenderInstanceData,
		HDynamicRenderInstance,
		XLinearArray<SDynamicRenderInstanceData>,
		SGPUDynamicRenderInstanceLayout
	>::Exists(go.dynamicRenderInstance))
		return {};

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
		batch = *batchSubsystem->Create(ERenderInstanceMotionType::Dynamic, geometry);
	}
	else
	{
		batch = *existingBatch;
	}

	HDynamicRenderInstance ri;

	ri = batchSubsystem->AddDynamicInstance(batch, gameObject);

	if (existingMaterial.has_value())
	{
		batchSubsystem->CUploader<
			SDynamicRenderInstanceData,
			HDynamicRenderInstance,
			XLinearArray<SDynamicRenderInstanceData>,
			SGPUDynamicRenderInstanceLayout
		>::Get(ri).material = *existingMaterial;
	}

	return ri;
}

triton::HDynamicRenderInstance triton::XGameObjectSubsystem::SetRenderableDynamic(
	const HGameObject& gameObject,
	const HModel3D& model,
	const std::optional<HBatch>& existingBatch,
	const std::optional<HMaterial>& existingMaterial
)
{
	SGameObjectData& go = Get(gameObject);
	if (_context->GetSubsystem<XBatchSubsystem>()->CUploader<
		SDynamicRenderInstanceData,
		HDynamicRenderInstance,
		XLinearArray<SDynamicRenderInstanceData>,
		SGPUDynamicRenderInstanceLayout
	>::Exists(go.dynamicRenderInstance))
		return {};

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
		batch = *_context->GetSubsystem<XBatchSubsystem>()->Create(ERenderInstanceMotionType::Dynamic, geometry);
	}
	else
	{
		batch = *existingBatch;
	}

	HDynamicRenderInstance ri;

	ri = _context->GetSubsystem<XBatchSubsystem>()->AddDynamicInstance(batch, gameObject);

	_context->GetSubsystem<XBatchSubsystem>()->CUploader<
		SDynamicRenderInstanceData,
		HDynamicRenderInstance,
		XLinearArray<SDynamicRenderInstanceData>,
		SGPUDynamicRenderInstanceLayout
	>::Get(ri).skeleton = m3dd.skeleton;

	if (existingMaterial.has_value())
	{
		_context->GetSubsystem<XBatchSubsystem>()->CUploader<
			SDynamicRenderInstanceData,
			HDynamicRenderInstance,
			XLinearArray<SDynamicRenderInstanceData>,
			SGPUDynamicRenderInstanceLayout
		>::Get(ri).material = *existingMaterial;
	}
	else
	{
		_context->GetSubsystem<XBatchSubsystem>()->CUploader<
			SDynamicRenderInstanceData,
			HDynamicRenderInstance,
			XLinearArray<SDynamicRenderInstanceData>,
			SGPUDynamicRenderInstanceLayout
		>::Get(ri).material.Invalidate();
	}

	return ri;
}