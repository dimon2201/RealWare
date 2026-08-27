// capabilities.hpp

#pragma once

#include <string>
#include "math.hpp"
#include "graphics_available_backends_enum.hpp"
#include "graphics_device_type_enum.hpp"
#include "types.hpp"

namespace triton
{
    struct sCapabilities
    {
        struct sWindowDescriptor
        {
            std::string title = "Test app";
            cVector2 size = cVector2(640, 480);
            types::boolean fullscreen = types::K_FALSE;
        };

        sWindowDescriptor window;
        types::usize memoryAlignment = 64;
        types::usize maxPhysicsSceneCount = 16;
        types::usize maxPhysicsMaterialCount = 256;
        types::usize maxPhysicsActorCount = 8192;
        types::usize maxPhysicsControllerCount = 8;
        types::usize maxSoundCount = 65536;
        types::usize maxEventPerTypeCount = 8192;
        types::usize maxRenderOpaqueInstanceCount = 65536;
        types::usize maxRenderTransparentInstanceCount = 65536;
        types::usize maxRenderTextInstanceCount = 8192;
        types::usize maxRenderMaterialCount = 256;
        types::usize maxRenderLightCount = 256;
        types::usize maxRenderTextureAtlasTextureCount = 8192;
        types::usize maxRenderStaticInstanceCount = 32 * 1024;
        types::usize maxRenderDynamicInstanceCount = 32 * 1024;
        types::usize staticVertexBufferSize = 64 * 1024 * 1024;
        types::usize skinnedVertexBufferSize = 64 * 1024 * 1024;
        types::usize staticIndexBufferSize = 32 * 1024 * 1024;
        types::usize skinnedIndexBufferSize = 32 * 1024 * 1024;
        types::usize boneBufferSize = 64 * 1024 * 1024;
        types::usize hashTableChunkByteSize = 16 * 1024;
        types::usize hashTableMaxChunkCount = 256;
        types::usize hashTableSize = 4096;
        types::usize futureResultBufferByteSize = 16 * 1024;
        types::usize handleAllocatorObjectCount = 16 * 1024;
        types::usize maxSkinnedBoneCount = 64 * 1024;
        types::usize maxSkeletonCount = 64 * 1024;
        types::usize maxPhysicsTempBufferByteSize = 10 * 1024 * 1024;
        types::usize maxPhysicsSimulationBodyCount = 1024;
        types::usize maxPhysicsSimulationBodyMutexCount = 0;
        types::usize maxPhysicsSimulationBodyPairCount = 1024;
        types::usize maxPhysicsSimulationContactConstraintCount = 1024;
        types::usize maxRenderBatchCount = 4 * 1024;
        types::usize maxRenderInstanceCount = 4 * 1024;
        types::usize maxBoneCountPerVertex = 4;
        EAvailableGraphicsBackend graphicsBackend = EAvailableGraphicsBackend::Unknown;
        EGraphicsDeviceType preferredGraphicsDevice = EGraphicsDeviceType::Unknown;
        types::usize framesInFlight = 2;
    };
}