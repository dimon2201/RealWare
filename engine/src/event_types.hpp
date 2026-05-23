// event_types.hpp

#include <functional>

namespace triton
{
    class iObject;
    class cContext;
    class XDataBuffer;

    using EventFunction = ::std::function<void(iObject* self, cContext* context, XDataBuffer* data)>;

    enum class eEventType
    {
        NONE,

        KEY_PRESS,

        FRAME_BEGIN,
        FRAME_UPDATE,
        FRAME_END
    };
}