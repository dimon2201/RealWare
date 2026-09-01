// world_render_group_instance.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "world_render_group_struct.hpp"
#include "world_render_instance.hpp"

namespace triton
{
	class XRenderGroupInstance : public iObject
	{
		SRenderGroupData _group;
		types::usize _maxInstanceCount = 0;
		types::usize _instanceCount = 0;
		SObjectFrame<XRenderInstance::THandle> _instances;

	public:
		explicit XRenderGroupInstance(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		explicit XRenderGroupInstance(
			cContext* context,
			types::s32 poolIndex,
			types::usize maxInstanceCount
		);

		~XRenderGroupInstance() override = default;

		const XRenderInstance::THandle& AddInstance();

		void RemoveInstance(const XRenderInstance::THandle& instance);

		inline const XCamera::THandle& GetCamera() { return _group.camera; }

		inline ERenderInstanceMotionType GetMotionType() { return _group.motionType; }

		inline const SGeometryView& GetSharedGeometry() { return _group.sharedGeometry; }

		inline const XMaterial::THandle& GetSharedMaterial() { return _group.sharedMaterial; }

		inline const std::vector<XRenderInstance>& GetInstances() { return _instances; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}