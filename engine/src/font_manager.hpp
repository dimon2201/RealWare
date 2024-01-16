#pragma once

#include <unordered_map>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"

namespace realware
{
    namespace core
    {
        class cUserInput;
    }

    namespace render
    {
        class cRenderContext;
        struct sTexture;
    }

    namespace font
    {
        struct sFont
        {
            struct sGlyph
            {
                char Character;
                core::u32 Width;
                core::u32 Height;
                core::s32 Left;
                core::s32 Top;
                float AdvanceX;
                float AdvanceY;
                core::s32 AtlasXOffset;
                core::s32 AtlasYOffset;
                void* BitmapData;
            };

            FT_Face Font;
            core::usize GlyphCount;
            core::usize GlyphSize;
            core::usize MaxGlyphHeight;
            float OffsetWhitespace;
            float OffsetNewline;
            float OffsetSpace;
            std::unordered_map<char, sGlyph> Alphabet;
            render::sTexture* Atlas;
        };

        class mFont
        {

        public:
            mFont() {}
            ~mFont() {}

            void Init(render::cRenderContext* context);
            void Free();

            sFont* NewFont(const char* filename, core::usize glyphSize, float whitespaceOffset, float newlineOffset, float spaceOffset);
            void DeleteFont(sFont* font);
            const char* CyrillicStringInit(const char* str);
            void CyrillicStringFree(const char* str);

            float GetTextWidth(const sFont* font, const char* text);
            float GetTextHeight(const sFont* font, const char* text);
            core::usize GetCharacterCount(const char* text);
            core::usize GetNewlineCount(const char* text);

        private:
            void FillUnicodeTable();

            core::u16 m_unicode[256] = {};
            render::cRenderContext* m_context = nullptr;
            FT_Library m_lib;

        };
    }
}