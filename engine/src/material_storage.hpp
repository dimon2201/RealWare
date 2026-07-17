// material_storage.hpp

#pragma once

#include "object.hpp"
#include "material.hpp"
#include "handles.hpp"
#include "math.hpp"
#include "uploader.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;

	class XMaterialStorage : public CUploader<SMaterial, HMaterial, XLinearArray<SMaterial>, SGPUMaterialLayout>
	{
	protected:
		explicit XMaterialStorage(
			cContext* context,
			cGPUResource** resource,
			types::usize stagingBufferElementCount,
			types::boolean bNeedsPersistentGpuWrite
		) : CUploader(context, resource, stagingBufferElementCount, bNeedsPersistentGpuWrite) {}

		HMaterial CreateMaterial(
			const cVector4& diffuseColor,
			const HTexture& diffuseTexture,
			const HTexture& normalTexture,
			const HTexture& roughnessTexture,
			const HTexture& metallicTexture
		);

		void DestroyMaterial(const HMaterial& material);
	};
}