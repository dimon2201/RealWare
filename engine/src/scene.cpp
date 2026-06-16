// scene.cpp

#pragma once

#include "scene.hpp"
#include "context.hpp"

using namespace triton::ecs::components;
using namespace types;

triton::ecs::cScene::cScene(cContext* context, const std::string& name) : iObject(context), _name(name)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();

	sChunkAllocatorDescriptor cad = {};
	cad.chunkByteSize = caps->hashTableChunkByteSize;
	cad.maxChunkCount = caps->hashTableMaxChunkCount;
	cad.hashTableSize = caps->hashTableSize;

	_isEntityExist = _context->Create<cHashTable<entity, cSingleValue>>(_context, cad);
	_transformComponents = _context->Create<cComponentStorage<components::STransformComponent>>(_context);
	_renderInstanceComponents = _context->Create<cComponentStorage<components::SRenderInstanceComponent>>(_context);
	_renderInstancesStatic = _context->Create<cStack<SRenderInstanceComponent>>(_context, cad);
	_renderInstancesDynamic = _context->Create<cStack<SRenderInstanceComponent>>(_context, cad);
}

triton::ecs::cScene::~cScene()
{
	_context->Destroy<cStack<SRenderInstanceComponent>>(_renderInstancesDynamic);
	_context->Destroy<cStack<SRenderInstanceComponent>>(_renderInstancesStatic);
	_context->Destroy<cComponentStorage<components::SRenderInstanceComponent>>(_renderInstanceComponents);
	_context->Destroy<cComponentStorage<components::STransformComponent>>(_transformComponents);
	_context->Destroy<cHashTable<entity, cSingleValue>>(_isEntityExist);
}

triton::ecs::entity triton::ecs::cScene::CreateEntity()
{
	entity ent = sLastEntity++;

	_isEntityExist->Insert(ent, K_TRUE);

	return ent;
}

void triton::ecs::cScene::DestroyEntity(entity ent)
{
	if (ent != kInvalidEntity)
		_isEntityExist->Erase(ent);
}

types::boolean triton::ecs::cScene::IsEntityExist(entity ent)
{
	cSingleValue* result = _isEntityExist->Find(ent);
	if (result == nullptr)
		return K_FALSE;
	else
		return result->Value();
}

triton::ecs::components::STransformComponent* triton::ecs::cScene::CreateTransformComponent(entity ent)
{
	return _transformComponents->Create(ent);
}

triton::ecs::components::SRenderInstanceComponent* triton::ecs::cScene::CreateRenderInstanceComponent(entity ent)
{
	return _renderInstanceComponents->Create(ent);
}

void triton::ecs::cScene::DestroyTransformComponent(entity ent)
{
	return _transformComponents->Destroy(ent);
}

void triton::ecs::cScene::DestroyRenderInstanceComponent(entity ent)
{
	return _renderInstanceComponents->Destroy(ent);
}

triton::SRenderData triton::ecs::cScene::BuildRenderDataStatic()
{
	_renderInstancesStatic->Clear();

	for (usize i = 0; i < _renderInstanceComponents->GetCount(); i++)
	{
		SRenderInstanceComponent* pRic = _renderInstanceComponents->Get(i);
		if (!pRic)
			continue;
		SRenderInstanceComponent ric = *pRic;
		if (ric._usage == SRenderInstance::EUsage::STATIC)
			_renderInstancesStatic->Push(std::move(ric));
	}

	SRenderData rd = {};
	rd._renderInstances = _renderInstancesStatic;

	return rd;
}

triton::SRenderData triton::ecs::cScene::BuildRenderDataDynamic()
{
	_renderInstancesDynamic->Clear();

	for (usize i = 0; i < _renderInstanceComponents->GetCount(); i++)
	{
		SRenderInstanceComponent* pRic = _renderInstanceComponents->Get(i);
		if (!pRic)
			continue;
		SRenderInstanceComponent ric = *pRic;
		if (ric._usage == SRenderInstance::EUsage::DYNAMIC)
			_renderInstancesDynamic->Push(std::move(ric));
	}

	SRenderData rd = {};
	rd._renderInstances = _renderInstancesDynamic;

	return rd;
}