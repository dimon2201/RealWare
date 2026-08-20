// render_instance.hpp

#pragma once

#include "object.hpp"
#include "render_instance_motion_type.hpp"
#include "material.hpp"
#include "math.hpp"
#include "skin.hpp"
#include "skin_pool.hpp"
#include "types.hpp"

namespace triton
{
	class XRenderInstance : public iObject
	{
		TRITON_OBJECT(XRenderInstance)

        ERenderInstanceMotionType   _usage = ERenderInstanceMotionType::Static;
        types::s32                  _indexInInstancePack = -1;
        XMaterial::THandle          _material;
        types::s32                  _skinnedBoneBufferOffset = 0;
        cMatrix4                    _worldMatrix = cMatrix4();

	public:
		explicit XRenderInstance(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

        ~XRenderInstance() override = default;

        inline types::s32 GetIndexInInstancePack() const { return _indexInInstancePack; }

        inline const XMaterial::THandle& GetMaterial() const { return _material; }

        inline types::s32 GetSkinnedBoneBufferOffset() const { return _skinnedBoneBufferOffset; }

        inline const cMatrix4& GetWorldMatrix() const { return _worldMatrix; }

        inline void SetIndexInInstancePack(types::s32 indexInInstancePack) { _indexInInstancePack = indexInInstancePack; }

        inline void SetMaterial(const XMaterial::THandle& material) { _material = material; }

        inline void SetSkin(const XSkin::THandle& skin)
        {
            XSkin& sk = *_context->GetPool<CSkinPool>()->Get(skin);
            _skinnedBoneBufferOffset = sk.GetSkinnedBoneBufferOffset();
        }

        struct THandle : public SHandle {};

        struct TGPULayout
        {
            types::f32 _use2D = 0.0f;
            types::s32 _materialIndex = -1;
            types::s32 _skinnedBoneBufferOffset = -1;
            types::dword _propertyBits = 0;
            cMatrix4 _world = cMatrix4(1.0f);
        };
	};
}