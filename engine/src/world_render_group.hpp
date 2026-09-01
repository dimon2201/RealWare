// world_render_group.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "world_render_group_struct.hpp"
#include "world_render_group_instance.hpp"

namespace triton
{
	class XRenderGroup : public iObject
	{
		TRITON_CLASS_NAME(XRenderGroup)

		SRenderGroupData _group;

	public:
		explicit XRenderGroup(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		explicit XRenderGroup(
			cContext* context,
			types::s32 poolIndex,
			const SHandle& renderDomain,
			ERenderInstanceMotionType motionType,
			const SGeometryView& sharedGeometry,
			const XMaterial::THandle& sharedMaterial
		) :
			iObject(context, poolIndex),
			_group(renderDomain, motionType, sharedGeometry, sharedMaterial) {}

		~XRenderGroup() override = default;

		XRenderGroupInstance::THandle CreateInstance(types::usize maxInstanceCount);

		void DestroyInstance(const XRenderGroupInstance::THandle& instance);

		inline const SHandle& GetRenderDomain() const { return _group.renderDomain; }

		inline ERenderInstanceMotionType GetMotionType() const { return _group.motionType; }

		inline const SGeometryView& GetSharedGeometry() const { return _group.sharedGeometry; }

		inline const XMaterial::THandle& GetSharedMaterial() const { return _group.sharedMaterial; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}