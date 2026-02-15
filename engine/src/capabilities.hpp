#pragma once

#include <string>
#include "platform.hpp"
#include "types.hpp"

namespace triton
{
    struct sCapabilities
    {
        struct sWindowDescriptor
        {
            std::string windowTitle = "Test app";
            types::usize windowWidth = 640;
            types::usize windowHeight = 480;
            types::boolean fullscreen = types::K_FALSE;
        };

        static constexpr types::usize kMaxWindowCount = 16;

        types::usize windowCount = 0;
        sWindowDescriptor windows[kMaxWindowCount];
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
        types::usize vertexBufferSize = 64 * 1024 * 1024;
        types::usize indexBufferSize = 64 * 1024 * 1024;
        types::usize hashTableChunkByteSize = 16 * 1024;
        types::usize hashTableMaxChunkCount = 256;
        types::usize hashTableSize = 4096;
    };
}