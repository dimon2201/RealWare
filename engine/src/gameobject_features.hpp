#pragma once

namespace realware
{
    namespace game
    {
        enum class GameObjectFeatures
        {
            PHYSICS_ACTOR_STATIC = 0,
            PHYSICS_ACTOR_DYNAMIC = 1,
            PHYSICS_SHAPE_PLANE = 2,
            PHYSICS_SHAPE_BOX = 3,
            VERTEX_BUFFER_FORMAT_NONE = 4,
            VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3 = 5,
            SOUND_FORMAT_WAV = 6
        };
    }
}