// game_object_subsystem.cpp

#include "game_object_subsystem.hpp"
#include "batch_storage.hpp"
#include "graphics.hpp"
#include "material_subsystem.hpp"

using namespace types;

void triton::XGameObjectSubsystem::Init()
{
}

void triton::XGameObjectSubsystem::Free()
{
}

void triton::XGameObjectSubsystem::Update()
{
	WriteDirty();
}

void triton::XGameObjectSubsystem::AddRenderable(const HGameObject& gameObject, SRenderInstance::EUsage usage, EGraphicsBufferFormat format, const types::u8* vertexBytes, types::usize vertexBytesCount, const types::u8* indexBytes, types::usize indexBytesCount)
{
	SGeometryView geometry = *_context->GetSubsystem<cGraphics>()->StoreGeometry(
		format,
		vertexBytes,
		vertexBytesCount,
		indexBytes,
		indexBytesCount
	);
	HBatch batchHandle = *_context->GetSubsystem<cGraphics>()->CreateBatch(geometry);
	SBatchInstance bi = *_context->GetSubsystem<cGraphics>()->CreateInstance(
		usage,
		batchHandle
	);
	SRenderInstance ri;
	
	SGameObject& go = *_objects->Get(gameObject);
	go.renderable = bi;

	AddDirty(bi, ri);
}

void triton::XGameObjectSubsystem::SetMaterial(const HGameObject& gameObject, const HMaterial& material)
{
	SGameObject& go = *_objects->Get(gameObject);
	go.material = material;

	SRenderInstance ri;
	ri._materialIndex = material._indexInArray;

	AddDirty(go.renderable, ri);
}

void triton::XGameObjectSubsystem::AddDirty(const SBatchInstance& batchInstance, const SRenderInstance& renderInstance)
{
	SDirtyBufferItem dbi;
	dbi.renderable = batchInstance;
	dbi.renderInstance = renderInstance;
	_dirtyBuffer.push_back(dbi);
}

void triton::XGameObjectSubsystem::WriteDirty()
{
	if (!_dirtyBuffer.empty())
	{
		for (auto& item : _dirtyBuffer)
		{
			// TODO: make batch storage as separate subsystem
			XRenderBatch* batch = _context->GetSubsystem<cGraphics>()->_batchStorage->Get(item.renderable.batch);
			batch->Set(item.renderable, item.renderInstance);
		}
		_dirtyBuffer.clear();
	}
}