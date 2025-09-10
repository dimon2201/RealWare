#include <iostream>
#include "font_manager.hpp"
#include "render_context.hpp"
#include "application.hpp"

namespace realware
{
    using namespace core;
    using namespace render;

    namespace font
    {
        mFont::mFont(const cApplication* const app, const cRenderContext* const context) : _app((cApplication*)app), _context((cRenderContext*)context)
        {
            if (FT_Init_FreeType(&_lib))
            {
                std::cout << "Failed to initialize FreeType library!" << std::endl;
                return;
            }

            _initialized = K_TRUE;
        }

        mFont::~mFont()
        {
            if (_initialized)
                FT_Done_FreeType(_lib);
        }

        usize CalculateNewlineOffset(const sFont* const  font)
        {
            return font->Font->size->metrics.height >> 6;
        }

        usize CalculateSpaceOffset(const sFont* const font)
        {
            const FT_UInt spaceIndex = FT_Get_Char_Index(font->Font, ' ');
            if (FT_Load_Glyph(font->Font, spaceIndex, FT_LOAD_DEFAULT) == 0)
                return font->Font->glyph->advance.x >> 6;
            else
                return 0;
        }

        void FillAlphabetAndFindAtlasSize(sFont* const font, usize& xOffset, usize& atlasWidth, usize& atlasHeight)
        {
            usize maxGlyphHeight = 0;

            for (int c = 0; c < 256; c++)
            {
                if (c == '\n' || c == ' ' || c == '\t')
                    continue;

                const FT_Int ci = FT_Get_Char_Index(font->Font, c);
                if (FT_Load_Glyph(font->Font, (FT_UInt)ci, FT_LOAD_DEFAULT) == 0)
                {
                    font->GlyphCount += 1;

                    FT_Render_Glyph(font->Font->glyph, FT_RENDER_MODE_NORMAL);

                    sFont::sGlyph glyph = {};
                    glyph.Character = (u8)c;
                    glyph.Width = font->Font->glyph->bitmap.width;
                    glyph.Height = font->Font->glyph->bitmap.rows;
                    glyph.Left = font->Font->glyph->bitmap_left;
                    glyph.Top = font->Font->glyph->bitmap_top;
                    glyph.AdvanceX = font->Font->glyph->advance.x >> 6;
                    glyph.AdvanceY = font->Font->glyph->advance.y >> 6;
                    glyph.BitmapData = malloc(glyph.Width * glyph.Height);

                    if (font->Font->glyph->bitmap.buffer)
                        memcpy(glyph.BitmapData, font->Font->glyph->bitmap.buffer, glyph.Width * glyph.Height);

                    font->Alphabet.insert({ (u8)c, glyph });

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

        void FillAtlasWithGlyphs(sFont* const font, usize& atlasWidth, usize& atlasHeight, cRenderContext* const context)
        {
            usize maxGlyphHeight = 0;

            void* const atlasPixels = malloc(atlasWidth * atlasHeight);
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
                (core::s32)atlasWidth,
                (core::s32)atlasHeight,
                0,
                render::sTexture::eType::TEXTURE_2D,
                render::sTexture::eFormat::R8,
                atlasPixels
            );

            free(atlasPixels);
        }

        sFont* mFont::CreateFontTTF(const std::string& filename, const usize glyphSize)
        {
            sFont* const font = new sFont();

            if (FT_New_Face(_lib, filename.c_str(), 0, &font->Font) == 0)
            {
                FT_Select_Charmap(font->Font, FT_ENCODING_UNICODE);

                if (FT_Set_Pixel_Sizes(font->Font, glyphSize, glyphSize) == 0)
                {
                    font->GlyphCount = 0;
                    font->GlyphSize = glyphSize;
                    font->OffsetNewline = CalculateNewlineOffset(font);
                    font->OffsetSpace = CalculateSpaceOffset(font);
                    font->OffsetTab = font->OffsetSpace * 4;

                    usize atlasWidth = 0;
                    usize atlasHeight = 0;
                    usize xOffset = 0;

                    FillAlphabetAndFindAtlasSize(font, xOffset, atlasWidth, atlasHeight);
                    MakeAtlasSizePowerOf2(atlasWidth, atlasHeight);
                    FillAtlasWithGlyphs(font, atlasWidth, atlasHeight, _context);
                }
                else
                {
                    delete font;
                    
                    return nullptr;
                }
            }
            else
            {
                std::cout << "Error creating FreeType font face!" << std::endl;
                delete font;
                
                return nullptr;
            }

            return font;
        }

        void mFont::DestroyFontTTF(sFont* const font)
        {
            for (auto glyph : font->Alphabet)
                free(glyph.second.BitmapData);

            font->Alphabet.clear();

            _context->DestroyTexture(font->Atlas);

            FT_Done_Face(font->Font);

            delete font->Atlas;
            delete font;
        }

        f32 mFont::GetTextWidth(const sFont* const font, const std::string& text)
        {
            f32 textWidth = 0.0f;
            f32 maxTextWidth = 0.0f;
            const usize textByteSize = strlen(text.c_str());
            const glm::vec2 windowSize = _app->GetWindowSize();

            for (usize i = 0; i < textByteSize; i++)
            {
                const sFont::sGlyph& glyph = font->Alphabet.find(text[i])->second;

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

        f32 mFont::GetTextHeight(const sFont* font, const std::string& text)
        {
            f32 textHeight = 0.0f;
            f32 maxHeight = 0.0f;
            const usize textByteSize = strlen(text.c_str());
            const glm::vec2 windowSize = _app->GetWindowSize();

            for (s32 i = 0; i < textByteSize; i++)
            {
                const sFont::sGlyph& glyph = font->Alphabet.find(text[i])->second;

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