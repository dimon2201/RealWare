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

        struct cFont
        {
        public:
            cFont() = default;
            ~cFont() = default;

            inline FT_Face& GetFont() { return _font; }
            inline types::usize GetGlyphCount() const { return _glyphCount; }
            inline types::usize GetGlyphSize() const { return _glyphSize; }
            inline types::usize GetNewlineOffset() const { return _offsetNewline; }
            inline types::usize GetSpaceOffset() const { return _offsetSpace; }
            inline types::usize GetTabOffset() const { return _offsetTab; }
            inline std::unordered_map<types::u8, sGlyph>& GetAlphabet() { return _alphabet; }
            inline render::sTexture* GetAtlas() const { return _atlas; }

            inline void SetGlyphCount(const types::usize value) { _glyphCount = value; }
            inline void SetGlyphSize(const types::usize value) { _glyphSize = value; }
            inline void SetNewlineOffset(const types::usize value) { _offsetNewline = value; }
            inline void SetSpaceOffset(const types::usize value) { _offsetSpace = value; }
            inline void SetTabOffset(const types::usize value) { _offsetTab = value; }
            inline void SetAtlas(const render::sTexture* const atlas) { _atlas = (render::sTexture*)atlas; }

        private:
            FT_Face _font = {};
            types::usize _glyphCount = 0;
            types::usize _glyphSize = 0;
            types::usize _offsetNewline = 0;
            types::usize _offsetSpace = 0;
            types::usize _offsetTab = 0;
            std::unordered_map<types::u8, sGlyph> _alphabet = {};
            render::sTexture* _atlas = nullptr;
        };

        class cText
        {
        public:
            explicit cText(const cFont* const font, const std::string& text) : _font((cFont*)font), _text(text) {}
            ~cText() = default;

            inline cFont* GetFont() const { return _font; }
            inline std::string GetText() const { return _text; }

        private:
            cFont* _font = nullptr;
            std::string _text = "";
        };

        class mFont
        {
        public:
            mFont(const app::cApplication* const app, const render::cRenderContext* const context);
            ~mFont();

            cFont* CreateFontTTF(const std::string& filename, const types::usize glyphSize);
            cText* CreateText(const cFont* const font, const std::string& text);
            void DestroyFontTTF(cFont* font);
            void DestroyText(cText* text);
            
            types::f32 GetTextWidth(cFont* const font, const std::string& text);
            types::f32 GetTextHeight(cFont* const font, const std::string& text);
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