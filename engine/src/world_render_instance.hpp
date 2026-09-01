// world_render_instance.hpp

#pragma once

#include "object.hpp"
#include "material.hpp"
#include "math.hpp"
#include "skin.hpp"
#include "skin_pool.hpp"
#include "types.hpp"

namespace triton
{
	class XRenderInstance : public iObject
	{
		TRITON_CLASS_NAME(XRenderInstance)

        types::s32                  _indexInGroup = -1;
        XMaterial::THandle          _material;
        types::s32                  _skinnedBoneBufferOffset = -1;
        cTransform                  _transform;

	public:
		explicit XRenderInstance(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

        explicit XRenderInstance(
            cContext* context,
            types::s32 poolIndex,
            types::s32 indexInGroup
        ) : iObject(context, poolIndex), _indexInGroup(indexInGroup) {}

        ~XRenderInstance() override = default;

        inline void Transform() { _transform.Transform(); }

        inline types::s32 GetIndexInGroup() const { return _indexInGroup; }

        inline const XMaterial::THandle& GetMaterial() const { return _material; }

        inline types::s32 GetSkinnedBoneBufferOffset() const { return _skinnedBoneBufferOffset; }

        inline const cVector3& GetWorldPosition() const { return _transform.GetPosition(); }

        inline const cVector3& GetRotation() const { return _transform.GetRotation(); }

        inline const cVector3& GetScale() const { return _transform.GetScale(); }

        inline const cMatrix4& GetWorldMatrix() const { return _transform.GetWorld(); }

        inline void SetMaterial(const XMaterial::THandle& material) { _material = material; }

        inline void SetSkin(const XSkin::THandle& skin)
        {
            XSkin& sk = *_context->GetPool<CSkinPool>()->Get(skin);
            _skinnedBoneBufferOffset = sk.GetSkinnedBoneBufferOffset();
        }

        inline void SetWorldPosition(const cVector3& worldPosition) { _transform.SetPosition(worldPosition); }

        inline void SetRotation(const cVector3& rotation) { _transform.SetRotation(rotation); }

        inline void SetScale(const cVector3& scale) { _transform.SetScale(scale); }
        
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