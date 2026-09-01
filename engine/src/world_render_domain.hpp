// world_render_domain.hpp

#pragma once

#include "camera.hpp"
#include "object.hpp"

namespace triton
{
	class XRenderDomain : public iObject
	{
		XCamera::THandle _camera;

	public:
		explicit XRenderDomain(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		~XRenderDomain() override = default;

		inline const XCamera::THandle& GetCamera() { return _camera; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}