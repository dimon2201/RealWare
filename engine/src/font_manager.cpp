// font_manager.cpp

#include <iostream>
#include "font_manager.hpp"
#include "application.hpp"
#include "memory_pool.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "engine.hpp"
#include "log.hpp"
#include "graphics_backend.hpp"
#include "dynamic_array.hpp"

using namespace types;

usize CalculateNewlineOffset(triton::cFontFace* font)
{
    return font->GetFont()->size->metrics.height >> 6;
}

usize CalculateSpaceOffset(triton::cFontFace* font)
{
    const FT_Face ftFont = font->GetFont();
    const FT_UInt spaceIndex = FT_Get_Char_Index(ftFont, ' ');
    if (FT_Load_Glyph(ftFont, spaceIndex, FT_LOAD_DEFAULT) == 0)
        return ftFont->glyph->advance.x >> 6;
    else
        return 0;
}

usize NextPowerOfTwo(usize n)
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

triton::cFontFace::cFontFace(cContext* context) : iObject(context) {}

triton::cFontFace::~cFontFace()
{
    cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

    for (const auto& glyph : _alphabet)
        memoryAllocator->Deallocate(glyph.second._bitmapData);
    _alphabet.clear();

    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)&_atlas
    ));

    FT_Done_Face(_font);
}

void triton::cFontFace::FillAlphabetAndFindAtlasSize(usize& xOffset, usize& atlasWidth, usize& atlasHeight)
{
    const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
    cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

    const FT_Face ftFont = _font;
    usize maxGlyphHeight = 0;

    for (usize c = 0; c < 256; c++)
    {
        if (c == '\n' || c == ' ' || c == '\t')
            continue;

        const FT_Int ci = FT_Get_Char_Index(ftFont, c);
        if (FT_Load_Glyph(ftFont, (FT_UInt)ci, FT_LOAD_DEFAULT) == 0)
        {
            _glyphCount += 1;

            FT_Render_Glyph(ftFont->glyph, FT_RENDER_MODE_NORMAL);

            sGlyph glyph = {};
            glyph._character = (u8)c;
            glyph._width = ftFont->glyph->bitmap.width;
            glyph._height = ftFont->glyph->bitmap.rows;
            glyph._left = ftFont->glyph->bitmap_left;
            glyph._top = ftFont->glyph->bitmap_top;
            glyph._advanceX = ftFont->glyph->advance.x >> 6;
            glyph._advanceY = ftFont->glyph->advance.y >> 6;
            glyph._bitmapData = memoryAllocator->Allocate(glyph._width * glyph._height, caps->memoryAlignment);

            if (ftFont->glyph->bitmap.buffer)
                memcpy(glyph._bitmapData, ftFont->glyph->bitmap.buffer, glyph._width * glyph._height);

            _alphabet.insert({ (u8)c, glyph });

            xOffset += glyph._width + 1;

            if (atlasWidth < cFont::K_MAX_ATLAS_WIDTH - (glyph._width + 1))
                atlasWidth += glyph._width + 1;

            if (glyph._height > maxGlyphHeight)
                maxGlyphHeight = glyph._height;

            if (xOffset >= cFont::K_MAX_ATLAS_WIDTH)
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

void triton::cFontFace::FillAtlasWithGlyphs(usize& atlasWidth, usize& atlasHeight)
{
    const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
    cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

    usize maxGlyphHeight = 0;

    void* atlasPixels = memoryAllocator->Allocate(atlasWidth * atlasHeight, caps->memoryAlignment);
    memset(atlasPixels, 0, atlasWidth * atlasHeight);

    usize xOffset = 0;
    usize yOffset = 0;
    u8* pixelsU8 = (u8*)atlasPixels;

    for (auto& glyph : _alphabet)
    {
        glyph.second._atlasXOffset = xOffset;
        glyph.second._atlasYOffset = yOffset;

        for (usize y = 0; y < glyph.second._height; y++)
        {
            for (usize x = 0; x < glyph.second._width; x++)
            {
                const usize glyphPixelIndex = x + (y * glyph.second._width);
                const usize pixelIndex = (xOffset + x) + ((yOffset + y) * atlasWidth);

                if (glyphPixelIndex < glyph.second._width * glyph.second._height &&
                    pixelIndex < atlasWidth * atlasHeight)
                    pixelsU8[pixelIndex] = ((u8*)glyph.second._bitmapData)[glyphPixelIndex];
            }
        }

        xOffset += glyph.second._width + 1;
        if (glyph.second._height > maxGlyphHeight)
            maxGlyphHeight = glyph.second._height;

        if (xOffset >= cFont::K_MAX_ATLAS_WIDTH)
        {
            yOffset += maxGlyphHeight + 1;
            xOffset = 0;
            maxGlyphHeight = 0;
        }
    }

    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        atlasWidth,
        atlasHeight,
        0,
        (cpuword)ETextureDimension::Texture2D,
        (cpuword)ETextureFormat::R8,
        (cpuword)atlasPixels,
        0
    ));
    _atlas = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTexture>();

    memoryAllocator->Deallocate(atlasPixels);
}

triton::cText::cText(cContext* context) : iObject(context) {}

triton::cText::~cText() {}

triton::cFont::cFont(cContext* context) : iObject(context)
{
    if (FT_Init_FreeType(&_lib))
    {
        Print("Failed to initialize FreeType library!");
        return;
    }

    _initialized = K_TRUE;
}

triton::cFont::~cFont()
{
    if (_initialized)
        FT_Done_FreeType(_lib);
}

triton::cFontFace* triton::cFont::CreateFontTTF(const std::string& filename, usize glyphSize)
{
    cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
    cFontFace* font = _context->Create<cFontFace>(_context);

    FT_Face ftFont = font->GetFont();

    if (FT_New_Face(_lib, filename.c_str(), 0, &ftFont) == 0)
    {
        FT_Select_Charmap(ftFont, FT_ENCODING_UNICODE);

        if (FT_Set_Pixel_Sizes(ftFont, glyphSize, glyphSize) == 0)
        {
            font->SetGlyphSize(glyphSize);
            font->SetOffsetNewline(CalculateNewlineOffset(font));
            font->SetOffsetSpace(CalculateSpaceOffset(font));
            font->SetOffsetTab(font->GetOffsetSpace() * 4);

            usize atlasWidth = 0;
            usize atlasHeight = 0;
            usize xOffset = 0;

            font->FillAlphabetAndFindAtlasSize(xOffset, atlasWidth, atlasHeight);
            MakeAtlasSizePowerOf2(atlasWidth, atlasHeight);
            font->FillAtlasWithGlyphs(atlasWidth, atlasHeight);
        }
        else
        {
            _context->Destroy<cFontFace>(font);
                    
            return nullptr;
        }
    }
    else
    {
        Print("Error creating FreeType font face!");

        _context->Destroy<cFontFace>(font);
                
        return nullptr;
    }

    return font;
}

triton::cText* triton::cFont::CreateText(cFontFace* font, const std::string& text)
{
    cText* textObject = (cText*)_context->Create<cText>(_context);

    textObject->SetFont(font);
    textObject->SetText(text);

    return textObject;
}

void triton::cFont::DestroyFontTTF(cFontFace* font)
{
    _context->Destroy<cFontFace>(font);
}

void triton::cFont::DestroyText(cText* text)
{
    _context->Destroy<cText>(text);
}

f32 triton::cFont::GetTextWidth(cFontFace* font, const std::string& text) const
{
    cInput* input = _context->GetSubsystem<cInput>();

    f32 textWidth = 0.0f;
    f32 maxTextWidth = 0.0f;
    const usize textByteSize = strlen(text.c_str());
    const cVector2 windowSize = input->GetWindows()->at(0).GetSize();

    for (usize i = 0; i < textByteSize; i++)
    {
        const sGlyph& glyph = font->GetAlphabet().find(text[i])->second;

        if (text[i] == '\t')
        {
            textWidth += font->GetOffsetTab();
        }
        else if (text[i] == ' ')
        {
            textWidth += font->GetOffsetSpace();
        }
        else if (text[i] == '\n')
        {
            if (maxTextWidth < textWidth)
                maxTextWidth = textWidth;
            textWidth = 0.0f;
        }
        else
        {
            textWidth += ((f32)glyph._width / windowSize.GetX());
        }
    }

    if (maxTextWidth < textWidth)
    {
        maxTextWidth = textWidth;
        textWidth = 0.0f;
    }

    return maxTextWidth;
}

f32 triton::cFont::GetTextHeight(cFontFace* font, const std::string& text) const
{
    cInput* input = _context->GetSubsystem<cInput>();

    f32 textHeight = 0.0f;
    f32 maxHeight = 0.0f;
    const usize textByteSize = strlen(text.c_str());
    const cVector2 windowSize = input->GetWindows()->at(0).GetSize();

    for (usize i = 0; i < textByteSize; i++)
    {
        const sGlyph& glyph = font->GetAlphabet().find(text[i])->second;

        if (text[i] == '\n')
        {
            textHeight += font->GetOffsetNewline();
        }
        else
        {
            f32 glyphHeight = ((f32)glyph._height / windowSize.GetY());
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

usize triton::cFont::GetCharacterCount(const std::string& text) const
{
    return strlen(text.c_str());
}

usize triton::cFont::GetNewlineCount(const std::string& text) const
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