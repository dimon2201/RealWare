// render_pass_dispatch_enum.hpp

#pragma once

namespace triton
{
    enum class ERenderPassDispatch
    {
        None,
        Clear,
        Geometry,
        Text,
        Processing
    };
}