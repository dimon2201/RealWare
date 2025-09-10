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
        class cApplication;
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
                core::u8 Character;
                core::s32 Width;
                core::s32 Height;
                core::s32 Left;
                core::s32 Top;
                core::f32 AdvanceX;
                core::f32 AdvanceY;
                core::s32 AtlasXOffset;
                core::s32 AtlasYOffset;
                void* BitmapData;
            };

            FT_Face Font = {};
            core::usize GlyphCount = 0;
            core::usize GlyphSize = 0;
            core::usize OffsetNewline = 0;
            core::usize OffsetSpace = 0;
            core::usize OffsetTab = 0;
            std::unordered_map<core::u8, sGlyph> Alphabet = {};
            render::sTexture* Atlas = nullptr;
        };

        class cText
        {
        public:
            explicit cText(const sFont* const font, const std::string& text) : m_font((sFont*)font), m_text(text) {}
            ~cText() = default;

            inline sFont* GetFont() const { return m_font; }
            inline std::string GetText() const { return m_text; }

        private:
            sFont* m_font = nullptr;
            std::string m_text = "";
        };

        class mFont
        {
        public:
            mFont(const core::cApplication* const app, const render::cRenderContext* const context);
            ~mFont();

            sFont* CreateFontTTF(const std::string& filename, const core::usize glyphSize);
            void DestroyFontTTF(sFont* const font);
            
            core::f32 GetTextWidth(const sFont* const font, const std::string& text);
            core::f32 GetTextHeight(const sFont* const font, const std::string& text);
            core::usize GetCharacterCount(const std::string& text);
            core::usize GetNewlineCount(const std::string& text);

            static constexpr core::usize MAX_ATLAS_WIDTH = 2048;

        private:
            core::cApplication* _app = nullptr;
            core::boolean _initialized = core::K_FALSE;
            core::u16 _unicode[256] = {};
            render::cRenderContext* _context = nullptr;
            FT_Library _lib = {};

        };
    }
}