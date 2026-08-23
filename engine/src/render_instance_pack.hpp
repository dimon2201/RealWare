// render_instance_pack.hpp

#pragma once

#include <vector>
#include <optional>
#include "object.hpp"
#include "geometry_view.hpp"
#include "render_instance_motion_type.hpp"
#include "handle.hpp"
#include "vertex_buffer_format.hpp"
#include "render_instance.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;

	class XRenderInstancePack : public iObject
	{
		TRITON_OBJECT(XRenderInstancePack)

		ERenderInstanceMotionType				_motionType = ERenderInstanceMotionType::Unknown;
		SGeometryView							_sharedGeometry = {};
		XMaterial::THandle						_sharedMaterial;
		types::usize							_bufferOffset = 0;
		types::usize							_instanceCount = 0;
		types::usize							_maxInstanceCount = 0;
		SObjectFrame<XRenderInstance::THandle>	_frame;

	public:
		explicit XRenderInstancePack(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		explicit XRenderInstancePack(
			cContext* context,
			types::s32 poolIndex,
			ERenderInstanceMotionType motionType,
			const SGeometryView& sharedGeometry,
			const XMaterial::THandle& sharedMaterial,
			types::usize maxInstanceCount
		);

		~XRenderInstancePack() override;

		std::optional<XRenderInstance::THandle> AddInstance();

		void RemoveInstance(const XRenderInstance::THandle& instance);

		inline ERenderInstanceMotionType GetMotionType() const { return _motionType; }

		inline const SGeometryView& GetSharedGeometry() const { return _sharedGeometry; }

		inline const XMaterial::THandle& GetSharedMaterial() const { return _sharedMaterial; }

		inline types::usize GetBufferOffset() const { return _bufferOffset; }

		inline types::usize GetInstanceCount() const { return _instanceCount; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}