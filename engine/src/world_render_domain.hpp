// world_render_domain.hpp

#pragma once

#include <vector>
#include <optional>
#include "camera.hpp"
#include "object.hpp"
#include "world_render_group_instance.hpp"
#include "handle.hpp"

namespace triton
{
	class XRenderDomain : public iObject
	{
		TRITON_CLASS_NAME(XRenderDomain)

		XCamera::THandle _camera;
		std::vector<SHandle> _renderGroupInstances;

	public:
		explicit XRenderDomain(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		explicit XRenderDomain(
			cContext* context,
			types::s32 poolIndex,
			const XCamera::THandle& camera
		) : iObject(context, poolIndex), _camera(camera) {}

		~XRenderDomain() override = default;

		std::optional<SHandle> CreateRenderGroup(
			ERenderInstanceMotionType motionType,
			const SGeometryView& geometry,
			const SHandle& material
		);

		void RemoveRenderGroup(const SHandle& renderGroup);

		void QueryRenderGroupInstances();

		inline const std::vector<SHandle>& GetRenderGroupInstances() const { return _renderGroupInstances; }

		inline const XCamera::THandle& GetCamera() const { return _camera; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}