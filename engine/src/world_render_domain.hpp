// world_render_domain.hpp

#pragma once

#include <vector>
#include "camera.hpp"
#include "object.hpp"
#include "world_render_group_instance.hpp"

namespace triton
{
	class XRenderDomain : public iObject
	{
		TRITON_CLASS_NAME(XRenderDomain)

		XCamera::THandle _camera;

	public:
		explicit XRenderDomain(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		explicit XRenderDomain(
			cContext* context,
			types::s32 poolIndex,
			const XCamera::THandle& camera
		) : iObject(context, poolIndex), _camera(camera) {}

		~XRenderDomain() override = default;

		std::vector<XRenderGroupInstance::THandle> QueryRenderGroupInstances();

		inline const XCamera::THandle& GetCamera() const { return _camera; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}