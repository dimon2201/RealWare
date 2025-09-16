#include <iostream>
#include "font_manager.hpp"
#include "render_context.hpp"
#include "application.hpp"
#include "memory_pool.hpp"
#include "log.hpp"

using namespace types;

namespace realware
{
    using namespace app;
    using namespace render;
    using namespace log;
    using namespace utils;

    namespace font
    {
        mFont::mFont(const cApplication* const app, const cRenderContext* const context) : _app((cApplication*)app), _context((cRenderContext*)context)
        {
            if (FT_Init_FreeType(&_lib))
            {
                Print("Failed to initialize FreeType library!");
                return;
            }

            _initialized = K_TRUE;
        }

        mFont::~mFont()
        {
            if (_initialized)
                FT_Done_FreeType(_lib);
        }

        usize CalculateNewlineOffset(sFont* const font)
        {
            return font->Font->size->metrics.height >> 6;
        }

        usize CalculateSpaceOffset(sFont* const font)
        {
            const FT_Face& ftFont = font->Font;
            const FT_UInt spaceIndex = FT_Get_Char_Index(ftFont, ' ');
            if (FT_Load_Glyph(ftFont, spaceIndex, FT_LOAD_DEFAULT) == 0)
                return ftFont->glyph->advance.x >> 6;
            else
                return 0;
        }

        void FillAlphabetAndFindAtlasSize(cMemoryPool* const memoryPool, sFont* const font, usize& xOffset, usize& atlasWidth, usize& atlasHeight)
        {
            const FT_Face& ftFont = font->Font;
            usize maxGlyphHeight = 0;

            for (usize c = 0; c < 256; c++)
            {
                if (c == '\n' || c == ' ' || c == '\t')
                    continue;

                const FT_Int ci = FT_Get_Char_Index(ftFont, c);
                if (FT_Load_Glyph(ftFont, (FT_UInt)ci, FT_LOAD_DEFAULT) == 0)
                {
                    font->GlyphCount += 1;

                    FT_Render_Glyph(ftFont->glyph, FT_RENDER_MODE_NORMAL);

                    sGlyph glyph = {};
                    glyph.Character = (u8)c;
                    glyph.Width = ftFont->glyph->bitmap.width;
                    glyph.Height = ftFont->glyph->bitmap.rows;
                    glyph.Left = ftFont->glyph->bitmap_left;
                    glyph.Top = ftFont->glyph->bitmap_top;
                    glyph.AdvanceX = ftFont->glyph->advance.x >> 6;
                    glyph.AdvanceY = ftFont->glyph->advance.y >> 6;
                    glyph.BitmapData = memoryPool->Allocate(glyph.Width * glyph.Height);

                    if (ftFont->glyph->bitmap.buffer)
                        memcpy(glyph.BitmapData, ftFont->glyph->bitmap.buffer, glyph.Width * glyph.Height);

                    font->Alphabet.insert({(u8)c, glyph});

                    xOffset += glyph.Width + 1;

                    if (atlasWidth < mFont::MAX_ATLAS_WIDTH - (glyph.Width + 1))
                        atlasWidth += glyph.Width + 1;

                    if (glyph.Height > maxGlyphHeight)
                        maxGlyphHeight = glyph.Height;

                    if (xOffset >= mFont::MAX_ATLAS_WIDTH)
                    {
                        atlasHeight += maxGlyphHeight + 1;
                        xOffset = 0;
                        maxGlyphHeight = 0;
                    }
                }
            }

            if (atlasHeight < maxGlyphHeight + 1)
                atlasHeight += maxGlyphHeight + 1;
        }

        usize NextPowerOfTwo(const usize n)
        {
            if (n <= 0)
                return 1;

            usize power = 1;
            while (power < n)
            {
                if (power >= 0x80000000)
                    return 1;

                power <<= 1;
            }

            return power;
        }

        void MakeAtlasSizePowerOf2(usize& atlasWidth, usize& atlasHeight)
        {
            atlasWidth = NextPowerOfTwo(atlasWidth);
            atlasHeight = NextPowerOfTwo(atlasHeight);
        }

        void FillAtlasWithGlyphs(cMemoryPool* const memoryPool, sFont* const font, usize& atlasWidth, usize& atlasHeight, cRenderContext* const context)
        {
            usize maxGlyphHeight = 0;

            void* const atlasPixels = memoryPool->Allocate(atlasWidth * atlasHeight);
            memset(atlasPixels, 0, atlasWidth * atlasHeight);

            usize xOffset = 0;
            usize yOffset = 0;
            u8* const pixelsU8 = (u8* const)atlasPixels;

            for (auto& glyph : font->Alphabet)
            {
                glyph.second.AtlasXOffset = xOffset;
                glyph.second.AtlasYOffset = yOffset;

                for (usize y = 0; y < glyph.second.Height; y++)
                {
                    for (usize x = 0; x < glyph.second.Width; x++)
                    {
                        const usize glyphPixelIndex = x + (y * glyph.second.Width);
                        const usize pixelIndex = (xOffset + x) + ((yOffset + y) * atlasWidth);
                        
                        if (glyphPixelIndex < glyph.second.Width * glyph.second.Height &&
                            pixelIndex < atlasWidth * atlasHeight)
                            pixelsU8[pixelIndex] = ((u8*)glyph.second.BitmapData)[glyphPixelIndex];
                    }
                }

                xOffset += glyph.second.Width + 1;
                if (glyph.second.Height > maxGlyphHeight)
                    maxGlyphHeight = glyph.second.Height;

                if (xOffset >= mFont::MAX_ATLAS_WIDTH)
                {
                    yOffset += maxGlyphHeight + 1;
                    xOffset = 0;
                    maxGlyphHeight = 0;
                }
            }

            font->Atlas = context->CreateTexture(
                atlasWidth,
                atlasHeight,
                0,
                render::sTexture::eType::TEXTURE_2D,
                render::sTexture::eFormat::R8,
                atlasPixels
            );

            memoryPool->Free(atlasPixels);
        }

        sFont* mFont::CreateFontTTF(const std::string& filename, const usize glyphSize)
        {
            cMemoryPool* const memoryPool = _app->GetMemoryPool();
            sFont* pFont = (sFont*)memoryPool->Allocate(sizeof(sFont));
            sFont* font = new (pFont) sFont;

            FT_Face& ftFont = font->Font;

            if (FT_New_Face(_lib, filename.c_str(), 0, &ftFont) == 0)
            {
                FT_Select_Charmap(ftFont, FT_ENCODING_UNICODE);

                if (FT_Set_Pixel_Sizes(ftFont, glyphSize, glyphSize) == 0)
                {
                    font->GlyphCount = 0;
                    font->GlyphSize = glyphSize;
                    font->OffsetNewline = CalculateNewlineOffset(font);
                    font->OffsetSpace = CalculateSpaceOffset(font);
                    font->OffsetTab = font->OffsetSpace * 4;

                    usize atlasWidth = 0;
                    usize atlasHeight = 0;
                    usize xOffset = 0;

                    FillAlphabetAndFindAtlasSize(memoryPool, font, xOffset, atlasWidth, atlasHeight);
                    MakeAtlasSizePowerOf2(atlasWidth, atlasHeight);
                    FillAtlasWithGlyphs(memoryPool, font, atlasWidth, atlasHeight, _context);
                }
                else
                {
                    font->~sFont();
                    _app->GetMemoryPool()->Free(font);
                    
                    return nullptr;
                }
            }
            else
            {
                Print("Error creating FreeType font face!");

                font->~sFont();
                _app->GetMemoryPool()->Free(font);
                
                return nullptr;
            }

            return font;
        }

        sText* mFont::CreateText(const sFont* const font, const std::string& text)
        {
            sText* pTextObject = (sText*)_app->GetMemoryPool()->Allocate(sizeof(sText));
            sText* textObject = new (pTextObject) sText;

            textObject->Font = (sFont*)font;
            textObject->Text = text;

            return textObject;
        }

        void mFont::DestroyFontTTF(sFont* font)
        {
            auto& alphabet = font->Alphabet;
            auto atlas = font->Atlas;

            for (auto& glyph : alphabet)
                _app->GetMemoryPool()->Free(glyph.second.BitmapData);

            alphabet.clear();

            _context->DestroyTexture(atlas);

            FT_Done_Face(font->Font);

            font->~sFont();
            _app->GetMemoryPool()->Free(font);
        }

        void mFont::DestroyText(sText* text)
        {
            text->~sText();
            _app->GetMemoryPool()->Free(text);
        }

        f32 mFont::GetTextWidth(sFont* const font, const std::string& text)
        {
            f32 textWidth = 0.0f;
            f32 maxTextWidth = 0.0f;
            const usize textByteSize = strlen(text.c_str());
            const glm::vec2 windowSize = _app->GetWindowSize();

            for (usize i = 0; i < textByteSize; i++)
            {
                const sGlyph& glyph = font->Alphabet.find(text[i])->second;

                if (text[i] == '\t')
                {
                    textWidth += font->OffsetTab;
                }
                else if (text[i] == ' ')
                {
                    textWidth += font->OffsetSpace;
                }
                else if (text[i] == '\n')
                {
                    if (maxTextWidth < textWidth)
                        maxTextWidth = textWidth;
                    textWidth = 0.0f;
                }
                else
                {
                    textWidth += ((f32)glyph.Width / windowSize.x);
                }
            }

            if (maxTextWidth < textWidth)
            {
                maxTextWidth = textWidth;
                textWidth = 0.0f;
            }

            return maxTextWidth;
        }

        f32 mFont::GetTextHeight(sFont* font, const std::string& text)
        {
            f32 textHeight = 0.0f;
            f32 maxHeight = 0.0f;
            const usize textByteSize = strlen(text.c_str());
            const glm::vec2 windowSize = _app->GetWindowSize();

            for (usize i = 0; i < textByteSize; i++)
            {
                const sGlyph& glyph = font->Alphabet.find(text[i])->second;

                if (text[i] == '\n')
                {
                    textHeight += font->OffsetNewline;
                }
                else
                {
                    f32 glyphHeight = ((f32)glyph.Height / windowSize.y);
                    if (glyphHeight > maxHeight) {
                        maxHeight = glyphHeight;
                    }
                }

                if (i == textByteSize - 1)
                {
                    textHeight += maxHeight;
                    maxHeight = 0.0f;
                }
            }

            return textHeight;
        }

        usize mFont::GetCharacterCount(const std::string& text)
        {
            return strlen(text.c_str());
        }

        usize mFont::GetNewlineCount(const std::string& text)
        {
            usize newlineCount = 0;
            const usize charCount = strlen(text.c_str());
            for (usize i = 0; i < charCount; i++)
            {
                if (text[i] == '\n')
                    newlineCount++;
            }

            return newlineCount;
        }
    }
}