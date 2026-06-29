// game_object_subsystem.cpp

#include "game_object_subsystem.hpp"
#include "batch_storage.hpp"
#include "graphics.hpp"

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

void triton::XGameObjectSubsystem::AddDirty(const SBatchInstance& renderable)
{
	SDirtyBufferItem dbi;
	dbi.renderable = renderable;
	_dirtyBuffer.push_back(dbi);
}

void triton::XGameObjectSubsystem::WriteDirty()
{
	if (!_dirtyBuffer.empty())
	{
		for (auto& item : _dirtyBuffer)
		{
			XRenderBatch* batch = _context->GetSubsystem<XBatchStorage>()->Get(item.renderable.batch);
			batch->Set(item.renderable, item.renderInstance);
		}
		_dirtyBuffer.clear();
	}
}