#include <iostream>
#include "font_manager.hpp"
#include "render_context.hpp"
#include "application.hpp"

namespace realware
{
    namespace font
    {
        mFont::mFont(cApplication* app, render::cRenderContext* context)
        {
            m_app = app;
            m_context = context;
            FillUnicodeTable();

            if (FT_Init_FreeType(&m_lib)) {
                std::cout << "Failed to initialize FreeType library!" << std::endl;
            }
        }

        mFont::~mFont()
        {
            FT_Done_FreeType(m_lib);
        }

        sFont* mFont::NewFont(const char* filename, core::usize glyphSize, float whitespaceOffset, float newlineOffset, float spaceOffset)
        {
            sFont* font = new sFont();

            if (FT_New_Face(m_lib, filename, 0, &font->Font) == 0)
            {
                FT_Select_Charmap(font->Font, FT_ENCODING_UNICODE);

                if (FT_Set_Pixel_Sizes(font->Font, glyphSize, glyphSize) == 0)
                {
                    font->GlyphCount = 0;
                    font->GlyphSize = glyphSize;
                    font->OffsetWhitespace = whitespaceOffset;
                    font->OffsetNewline = font->Font->ascender - font->Font->descender;
                    font->OffsetSpace = spaceOffset;

                    core::usize textureWidth = 0;
                    core::usize textureHeight = 0;
                    core::usize xOffset = 0;
                    core::usize maxGlyphHeight = 0;
                    int lastIndex = 0;

                    for (int c = 0; c < 256; c++)
                    {
                        FT_Int ci = FT_Get_Char_Index(font->Font, m_unicode[(char)c]);
                        if (FT_Load_Glyph(font->Font, (FT_UInt)ci, FT_LOAD_DEFAULT) == 0)
                        {
                            font->GlyphCount += 1;

                            FT_Render_Glyph(font->Font->glyph, FT_RENDER_MODE_NORMAL);

                            sFont::sGlyph glyph = {};
                            glyph.Character = (char)c;
                            glyph.Width = font->Font->glyph->bitmap.width;
                            glyph.Height = font->Font->glyph->bitmap.rows;
                            glyph.Left = font->Font->glyph->bitmap_left;
                            glyph.Top = font->Font->glyph->bitmap_top;
                            glyph.AdvanceX = font->Font->glyph->advance.x;
                            glyph.AdvanceY = font->Font->glyph->advance.y;
                            glyph.BitmapData = malloc(glyph.Width * glyph.Height);

                            memcpy(glyph.BitmapData, font->Font->glyph->bitmap.buffer, glyph.Width * glyph.Height);

                            font->Alphabet.insert({ (char)c, glyph });

                            xOffset += glyph.Width + 1;
                            if (textureWidth <= 512) { textureWidth += glyph.Width + 1; }

                            if (glyph.Height > maxGlyphHeight) {
                                maxGlyphHeight = glyph.Height;
                            }

                            if (xOffset > 512)
                            {
                                textureHeight += maxGlyphHeight + 1;
                                xOffset = 0;
                                maxGlyphHeight = 0;
                            }
                        }
                    }

                    textureHeight += maxGlyphHeight + 1;

                    core::s32 bit;
                    bit = 0;
                    while ((1 << bit++) < textureWidth);
                    textureWidth = 1 << (bit - 1);
                    bit = 0;
                    while ((1 << bit++) < textureHeight);
                    textureHeight = 1 << (bit - 1);

                    void* atlasPixels = malloc(textureWidth * textureHeight);
                    memset(atlasPixels, 0, textureWidth * textureHeight);

                    // Fill atlas texture with glyphs
                    xOffset = 0;
                    core::usize yOffset = 0;
                    unsigned char* pixelsU8 = (unsigned char*)atlasPixels;
                    for (auto& glyph : font->Alphabet)
                    {
                        glyph.second.AtlasXOffset = xOffset;
                        glyph.second.AtlasYOffset = yOffset;

                        for (int y = 0; y < glyph.second.Height; y++)
                        {
                            for (int x = 0; x < glyph.second.Width; x++)
                            {
                                int glyphPixelIndex = x + (y * glyph.second.Width);
                                int pixelIndex = (xOffset + x) + ((yOffset + y) * textureWidth);
                                pixelsU8[pixelIndex] = ((char*)glyph.second.BitmapData)[glyphPixelIndex];
                            }
                        }

                        xOffset += glyph.second.Width + 1;
                        if (glyph.second.Height > maxGlyphHeight) {
                            maxGlyphHeight = glyph.second.Height;
                        }

                        if (xOffset > 512)
                        {
                            yOffset += maxGlyphHeight + 1;
                            xOffset = 0;
                            maxGlyphHeight = 0;
                        }
                    }

                    // Find max glyph height
                    font->MaxGlyphHeight = 0;
                    for (auto& glyph : font->Alphabet) {
                        if (glyph.second.Height > font->MaxGlyphHeight) {
                            font->MaxGlyphHeight = glyph.second.Height;
                        }
                    }

                    font->Atlas = m_context->CreateTexture((core::s32)textureWidth, (core::s32)textureHeight, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::R8, atlasPixels);

                    free(atlasPixels);
                }
                else
                {
                    delete font;
                    font = nullptr;
                }
            }
            else
            {
                std::cout << "Error creating FreeType font face!" << std::endl;
                delete font;
                font = nullptr;
            }

            return font;
        }

        void mFont::DeleteFont(sFont* font)
        {
            font->Alphabet.clear();
            m_context->DeleteTexture(font->Atlas);
            FT_Done_Face(font->Font);
            delete font->Atlas;
            delete font;
        }

        const char* mFont::CyrillicStringInit(const char* str)
        {
            core::u8* strBytes = (core::u8*)str;
            core::usize strByteSize = strlen(str);
            core::usize strCyrByteSize = strByteSize;
            char* strCyr = (char*)malloc(strCyrByteSize);
            memset((void*)strCyr, 0, strCyrByteSize);

            for (core::s32 i = 0, j = 0; i < strByteSize;)
            {
                if (strBytes[i] == 208 || strBytes[i] == 209)
                {
                    i++;
                }
                else
                {
                    strCyr[j] = str[i];
                    i += 1;
                    j += 1;
                }
            }

            return (const char*)&strCyr[0];
        }

        void mFont::CyrillicStringFree(const char* str)
        {
            if (str != nullptr) {
                free((void*)str);
            }
        }

        float mFont::GetTextWidth(const sFont* font, const char* text)
        {
            float textWidth = 0.0f;
            float maxTextWidth = 0.0f;
            core::usize textByteSize = strlen(text);
            const glm::vec2 windowSize = m_app->GetWindowSize();

            for (core::s32 i = 0; i < textByteSize; i++)
            {
                const font::sFont::sGlyph& glyph = font->Alphabet.find(text[i])->second;

                if (text[i] == '\t')
                {
                    textWidth += font->OffsetWhitespace;
                }
                else if (text[i] == ' ')
                {
                    textWidth += font->OffsetSpace;
                }
                else if (text[i] == '\n')
                {
                    if (maxTextWidth < textWidth) {
                        maxTextWidth = textWidth;
                    }
                    textWidth = 0.0f;
                }
                else
                {
                    textWidth += ((float)glyph.Width / windowSize.x);
                }
            }

            if (maxTextWidth < textWidth)
            {
                maxTextWidth = textWidth;
                textWidth = 0.0f;
            }

            return maxTextWidth;
        }

        float mFont::GetTextHeight(const sFont* font, const char* text)
        {
            float textHeight = 0.0f;
            float maxHeight = 0.0f;
            core::usize textByteSize = strlen(text);
            const glm::vec2 windowSize = m_app->GetWindowSize();

            for (core::s32 i = 0; i < textByteSize; i++)
            {
                const font::sFont::sGlyph& glyph = font->Alphabet.find(text[i])->second;

                if (text[i] == '\n')
                {
                    textHeight += font->OffsetNewline;
                }
                else
                {
                    float glyphHeight = ((float)glyph.Height / windowSize.y);
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

        core::usize mFont::GetCharacterCount(const char* text)
        {
            return strlen(text);
        }

        core::usize mFont::GetNewlineCount(const char* text)
        {
            core::usize newlineCount = 0;
            core::usize charCount = strlen(text);
            for (core::usize i = 0; i < charCount; i++)
            {
                if (text[i] == '\n') {
                    newlineCount += 1;
                }
            }

            return newlineCount;
        }

        void mFont::FillUnicodeTable()
        {
            m_unicode[' '] = 20;
            m_unicode['+'] = 43; m_unicode[','] = 44; m_unicode['.'] = 46;
            m_unicode['0'] = 48; m_unicode['1'] = 49; m_unicode['2'] = 50; m_unicode['3'] = 51;
            m_unicode['4'] = 52; m_unicode['5'] = 53; m_unicode['6'] = 54; m_unicode['7'] = 55;
            m_unicode['8'] = 56; m_unicode['9'] = 57;
            m_unicode['A'] = 65; m_unicode['B'] = 66; m_unicode['C'] = 67; m_unicode['D'] = 68;
            m_unicode['E'] = 69; m_unicode['F'] = 70; m_unicode['G'] = 71; m_unicode['H'] = 72;
            m_unicode['I'] = 73; m_unicode['J'] = 74; m_unicode['K'] = 75; m_unicode['L'] = 76;
            m_unicode['M'] = 77; m_unicode['N'] = 78; m_unicode['O'] = 79; m_unicode['P'] = 80;
            m_unicode['Q'] = 81; m_unicode['R'] = 82; m_unicode['S'] = 83; m_unicode['T'] = 84;
            m_unicode['U'] = 85; m_unicode['V'] = 86; m_unicode['W'] = 87; m_unicode['X'] = 88;
            m_unicode['Y'] = 89; m_unicode['Z'] = 90; m_unicode['a'] = 97; m_unicode['b'] = 98;
            m_unicode['c'] = 99; m_unicode['d'] = 100; m_unicode['e'] = 101; m_unicode['f'] = 102;
            m_unicode['g'] = 103; m_unicode['h'] = 104; m_unicode['i'] = 105; m_unicode['j'] = 106;
            m_unicode['k'] = 107; m_unicode['l'] = 108; m_unicode['m'] = 109; m_unicode['n'] = 110;
            m_unicode['o'] = 111; m_unicode['p'] = 112; m_unicode['q'] = 113; m_unicode['r'] = 114;
            m_unicode['s'] = 115; m_unicode['t'] = 116; m_unicode['u'] = 117; m_unicode['v'] = 118;
            m_unicode['w'] = 119; m_unicode['x'] = 120; m_unicode['y'] = 121; m_unicode['z'] = 122;
            m_unicode["Ё"[1]] = 1025; m_unicode["А"[1]] = 1040; m_unicode["Б"[1]] = 1041; m_unicode["В"[1]] = 1042;
            m_unicode["Г"[1]] = 1043; m_unicode["Д"[1]] = 1044; m_unicode["Е"[1]] = 1045; m_unicode["Ж"[1]] = 1046;
            m_unicode["З"[1]] = 1047; m_unicode["И"[1]] = 1048; m_unicode["Й"[1]] = 1049; m_unicode["К"[1]] = 1050;
            m_unicode["Л"[1]] = 1051; m_unicode["М"[1]] = 1052; m_unicode["Н"[1]] = 1053; m_unicode["О"[1]] = 1054;
            m_unicode["П"[1]] = 1055; m_unicode["Р"[1]] = 1056; m_unicode["С"[1]] = 1057; m_unicode["Т"[1]] = 1058;
            m_unicode["У"[1]] = 1059; m_unicode["Ф"[1]] = 1060; m_unicode["Х"[1]] = 1061; m_unicode["Ц"[1]] = 1062;
            m_unicode["Ч"[1]] = 1063; m_unicode["Ш"[1]] = 1064; m_unicode["Щ"[1]] = 1065; m_unicode["Ъ"[1]] = 1066;
            m_unicode["Ы"[1]] = 1067; m_unicode["Ь"[1]] = 1068; m_unicode["Э"[1]] = 1069; m_unicode["Ю"[1]] = 1070;
            m_unicode["Я"[1]] = 1071; m_unicode["а"[1]] = 1072; m_unicode["б"[1]] = 1073; m_unicode["в"[1]] = 1074;
            m_unicode["г"[1]] = 1075; m_unicode["д"[1]] = 1076; m_unicode["е"[1]] = 1077; m_unicode["ж"[1]] = 1078;
            m_unicode["з"[1]] = 1079; m_unicode["и"[1]] = 1080; m_unicode["й"[1]] = 1081; m_unicode["к"[1]] = 1082;
            m_unicode["л"[1]] = 1083; m_unicode["м"[1]] = 1084; m_unicode["н"[1]] = 1085; m_unicode["о"[1]] = 1086;
            m_unicode["п"[1]] = 1087; m_unicode["р"[1]] = 1088; m_unicode["с"[1]] = 1089; m_unicode["т"[1]] = 1090;
            m_unicode["у"[1]] = 1091; m_unicode["ф"[1]] = 1092; m_unicode["х"[1]] = 1093; m_unicode["ц"[1]] = 1094;
            m_unicode["ч"[1]] = 1095; m_unicode["ш"[1]] = 1096; m_unicode["щ"[1]] = 1097; m_unicode["ъ"[1]] = 1098;
            m_unicode["ы"[1]] = 1099; m_unicode["ь"[1]] = 1100; m_unicode["э"[1]] = 1101; m_unicode["ю"[1]] = 1102;
            m_unicode["я"[1]] = 1103;
        }
    }
}