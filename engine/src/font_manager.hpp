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
        class iRenderContext;
        struct sTexture;
    }

    namespace font
    {
        struct sGlyph
        {
            types::u8 Character = 0;
            types::s32 Width = 0;
            types::s32 Height = 0;
            types::s32 Left = 0;
            types::s32 Top = 0;
            types::f32 AdvanceX = 0.0f;
            types::f32 AdvanceY = 0.0f;
            types::s32 AtlasXOffset = 0;
            types::s32 AtlasYOffset = 0;
            void* BitmapData = nullptr;
        };

        struct sFont
        {
            FT_Face Font = {};
            types::usize GlyphCount = 0;
            types::usize GlyphSize = 0;
            types::usize OffsetNewline = 0;
            types::usize OffsetSpace = 0;
            types::usize OffsetTab = 0;
            std::unordered_map<types::u8, sGlyph> Alphabet = {};
            render::sTexture* Atlas = nullptr;
        };

        struct sText
        {
            sFont* Font = nullptr;
            std::string Text = "";
        };

        class mFont
        {
        public:
            mFont(const app::cApplication* const app, const render::iRenderContext* const context);
            ~mFont();

            sFont* CreateFontTTF(const std::string& filename, const types::usize glyphSize);
            sText* CreateText(const sFont* const font, const std::string& text);
            void DestroyFontTTF(sFont* font);
            void DestroyText(sText* text);
            
            types::f32 GetTextWidth(sFont* const font, const std::string& text);
            types::f32 GetTextHeight(sFont* const font, const std::string& text);
            types::usize GetCharacterCount(const std::string& text);
            types::usize GetNewlineCount(const std::string& text);

            static constexpr types::usize K_MAX_ATLAS_WIDTH = 2048;

        private:
            app::cApplication* _app = nullptr;
            types::boolean _initialized = types::K_FALSE;
            render::iRenderContext* _context = nullptr;
            FT_Library _lib = {};
        };
    }
}