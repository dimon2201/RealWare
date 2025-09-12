#pragma once

#include <unordered_map>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
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
                types::u8 Character;
                types::s32 Width;
                types::s32 Height;
                types::s32 Left;
                types::s32 Top;
                types::f32 AdvanceX;
                types::f32 AdvanceY;
                types::s32 AtlasXOffset;
                types::s32 AtlasYOffset;
                void* BitmapData;
            };

            FT_Face Font = {};
            types::usize GlyphCount = 0;
            types::usize GlyphSize = 0;
            types::usize OffsetNewline = 0;
            types::usize OffsetSpace = 0;
            types::usize OffsetTab = 0;
            std::unordered_map<types::u8, sGlyph> Alphabet = {};
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
            mFont(const app::cApplication* const app, const render::cRenderContext* const context);
            ~mFont();

            sFont* CreateFontTTF(const std::string& filename, const types::usize glyphSize);
            void DestroyFontTTF(sFont* const font);
            
            types::f32 GetTextWidth(const sFont* const font, const std::string& text);
            types::f32 GetTextHeight(const sFont* const font, const std::string& text);
            types::usize GetCharacterCount(const std::string& text);
            types::usize GetNewlineCount(const std::string& text);

            static constexpr types::usize MAX_ATLAS_WIDTH = 2048;

        private:
            app::cApplication* _app = nullptr;
            types::boolean _initialized = types::K_FALSE;
            types::u16 _unicode[256] = {};
            render::cRenderContext* _context = nullptr;
            FT_Library _lib = {};

        };
    }
}