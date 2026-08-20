// skinned_bone.hpp

#pragma once

#include "object.hpp"
#include "handle.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
	class CSkinnedBone : public iObject
	{
		TRITON_CLASS_NAME(CSkinnedBone)

		cMatrix4 _modelMatrix = cMatrix4();

	public:
		CSkinnedBone(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}
		~CSkinnedBone() override = default;

		inline const cMatrix4& GetModelMatrix() const { return _modelMatrix; }

		inline void SetModelMatrix(const cMatrix4& modelMatrix) { _modelMatrix = modelMatrix; }

		struct THandle : public SHandle {};

		struct TGPULayout
		{
			cMatrix4 modelMatrix = cMatrix4();
		};
	};
}