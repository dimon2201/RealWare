// audio_backend_sound.hpp

#pragma once

#include "types.hpp"

namespace triton
{
    struct sAudioBackendSound
    {
        types::qword source = 0;
        types::qword buffer = 0;
    };
}