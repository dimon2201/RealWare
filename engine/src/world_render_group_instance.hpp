// world_render_group_instance.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "world_render_group_struct.hpp"
#include "world_render_instance.hpp"
#include "camera.hpp"
#include "render_instance_motion_type.hpp"
#include "geometry_view.hpp"
#include "material.hpp"

namespace triton
{
	class XRenderGroupInstance : public iObject
	{
		TRITON_CLASS_NAME(XRenderGroupInstance)
		
		SRenderGroupData _group;
		types::usize _maxInstanceCount = 0;
		types::usize _instanceCount = 0;
		types::usize _instanceBufferOffset = 0;
		SObjectFrame<XRenderInstance::THandle> _instances;

	public:
		explicit XRenderGroupInstance(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		explicit XRenderGroupInstance(
			cContext* context,
			types::s32 poolIndex,
			const SHandle& renderDomain,
			ERenderInstanceMotionType motionType,
			const SGeometryView& sharedGeometry,
			const XMaterial::THandle& sharedMaterial,
			types::usize maxInstanceCount
		);

		~XRenderGroupInstance() override;

		const XRenderInstance::THandle& AddInstance();

		void RemoveInstance(const XRenderInstance::THandle& instance);

		inline types::boolean IsFull() const { return _instanceCount >= _maxInstanceCount ? types::True : types::False; }

		inline const SHandle& GetRenderDomain() const { return _group.renderDomain; }

		inline ERenderInstanceMotionType GetMotionType() const { return _group.motionType; }

		inline const SGeometryView& GetSharedGeometry() const { return _group.sharedGeometry; }

		inline const XMaterial::THandle& GetSharedMaterial() const { return _group.sharedMaterial; }

		inline const SObjectFrame<XRenderInstance::THandle>& GetInstances() const { return _instances; }

		inline types::usize GetInstanceCount() const { return _instanceCount; }

		inline types::usize GetInstanceBufferOffset() const { return _instanceBufferOffset; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}