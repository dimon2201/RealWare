// world_object.hpp

#pragma once

#include "object.hpp"
#include "world_render_domain.hpp"
#include "world_render_group_instance.hpp"
#include "skeleton.hpp"
#include "skin.hpp"
#include "animation.hpp"

namespace triton
{
	class XWorldObject : public iObject
	{
		TRITON_CLASS_NAME(XWorldObject)

		XRenderDomain::THandle				_renderDomain;
		XRenderGroupInstance::THandle		_renderGroupInstance;
		XRenderInstance::THandle			_renderInstance;
		XSkeleton::THandle                  _skeleton;
		XSkin::THandle                      _skin;
		std::vector<XAnimation::THandle>    _animations = {};

	public:
		explicit XWorldObject(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		~XWorldObject() override = default;

		void SetRenderable(
			types::boolean bIsRenderable,
			const XRenderDomain::THandle& renderDomain = XRenderDomain::THandle(),
			ERenderInstanceMotionType motionType = ERenderInstanceMotionType::Unknown,
			const SGeometryView& geometry = SGeometryView(),
			const XMaterial::THandle& material = XMaterial::THandle()
		);

		void PlayAnimation(types::usize index);

		struct THandle : public SHandle {};

		struct TGPULayout {};

	private:
		std::optional<XRenderGroupInstance::THandle> FindRenderGroupInstance(
			const XRenderDomain::THandle& renderDomain,
			ERenderInstanceMotionType motionType,
			const SGeometryView& geometry,
			const XMaterial::THandle& material
		);

		types::boolean IsRenderGroupInstanceFound(
			const XRenderGroupInstance& compareRenderGroupInstance,
			const XRenderDomain::THandle& renderDomain,
			ERenderInstanceMotionType motionType,
			const SGeometryView& geometry,
			const XMaterial::THandle& material
		);
	};
}