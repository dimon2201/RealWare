// shader_define.hpp

#pragma once

#include <string>
#include "types.hpp"

namespace triton
{
    class SShaderDefine final
    {
    public:
        SShaderDefine(const std::string& name_, types::usize index_) : name(name_), index(index_) {}

        std::string name = "";
        types::usize index = 0;
    };
}