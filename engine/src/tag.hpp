// tag.hpp

#pragma once

#include <string>
#include <array>
#include "types.hpp"

namespace triton
{
    class cContext;
    class cIdentifier;
    
    class cTag
    {
        friend class cIdentifier;

    public:
        static constexpr types::usize kMaxTagByteSize = 32;
        using chars = ::std::array<types::u8, kMaxTagByteSize>;

    private:
        chars _data = {};
        types::usize _byteSize = 0;

        void FillZeros();
        void CopyChars(const types::u8* chars, types::usize charsByteSize);

    public:
        explicit cTag();
        explicit cTag(const ::std::string& text);
        explicit cTag(const types::u8* chars, types::usize charsByteSize);
        ~cTag() = default;

        bool operator==(const cTag& rhs);
        bool operator==(const ::std::string& rhs);

        inline const chars& GetData() const { return _data; }
        inline types::usize GetByteSize() const { return _byteSize; }
    };
}