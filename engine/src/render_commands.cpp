// render_commands.cpp

#pragma once

#include "render_commands.hpp"
#include "context.hpp"
#include "graphics_resource_backend.hpp"

void triton::CCreateTextureCommand::Execute(cContext* context)
{
	iGraphicsResourceBackend* gfxResourceBackend = context->GetBackend<iGraphicsResourceBackend>();
	cTexture* texture = gfxResourceBackend->CreateTexture(
		cVector3(_argA, _argB, _argC),
		(cTexture::eDimension)_argD,
		(cTexture::eFormat)_argE,
		(types::u8*)_argF,
		_argG
	);
	_promise.set_value(std::move(texture));
}

std::future<triton::cTexture*> triton::CCreateTextureCommand::GetFuture()
{
	return _promise.get_future();
}