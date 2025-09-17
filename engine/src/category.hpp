#pragma once

namespace realware
{
    namespace game
    {
        enum class Category
        {
            PHYSICS_ACTOR_STATIC = 0,
            PHYSICS_ACTOR_DYNAMIC = 1,
            PHYSICS_SHAPE_PLANE = 2,
            PHYSICS_SHAPE_BOX = 3,
            VERTEX_BUFFER_FORMAT_NONE = 4,
            VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3 = 5,
            SOUND_FORMAT_WAV = 6,
            CAMERA_ANGLE_PITCH = 7,
            CAMERA_ANGLE_YAW = 8,
            CAMERA_ANGLE_ROLL = 9,
            PRIMITIVE_TRIANGLE = 10,
            PRIMITIVE_QUAD = 11,
            RENDER_PATH_NONE = 12,
            RENDER_PATH_OPAQUE = 13,
            RENDER_PATH_TRANSPARENT = 14,
            RENDER_PATH_TEXT = 15,
            RENDER_PATH_TRANSPARENT_COMPOSITE = 16,
            RENDER_PATH_QUAD = 17
        };
    }
}