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

    using namespace core;

    namespace font
    {
        struct sFont
        {
            struct sGlyph
            {
                char Character;
                s32 Width;
                s32 Height;
                s32 Left;
                s32 Top;
                float AdvanceX;
                float AdvanceY;
                s32 AtlasXOffset;
                s32 AtlasYOffset;
                void* BitmapData;
            };

            FT_Face Font;
            usize GlyphCount;
            usize GlyphSize;
            float OffsetNewline;
            float OffsetSpace;
            float OffsetTab;
            std::unordered_map<char, sGlyph> Alphabet;
            render::sTexture* Atlas;
        };

        class cText
        {

        public:
            cText(sFont* font, const std::string& text)
            {
                m_font = font;
                m_text = text;
            }
            ~cText() = default;

            inline sFont* GetFont() { return m_font; }
            inline std::string GetText() { return m_text; }

        private:
            sFont* m_font = nullptr;
            std::string m_text = "";

        };

        class mFont
        {

        public:
            mFont(cApplication* app, render::cRenderContext* context);
            ~mFont();

            sFont* NewFont(const char* filename, usize glyphSize);
            void DeleteFont(sFont* font);
            const char* CyrillicStringInit(const char* str);
            void CyrillicStringFree(const char* str);

            float GetTextWidth(const sFont* font, const char* text);
            float GetTextHeight(const sFont* font, const char* text);
            usize GetCharacterCount(const char* text);
            usize GetNewlineCount(const char* text);

        private:
            void FillUnicodeTable();

            cApplication* m_app;
            boolean _initialized = K_FALSE;
            u16 m_unicode[256] = {};
            render::cRenderContext* m_context = nullptr;
            FT_Library m_lib;

        };
    }
}