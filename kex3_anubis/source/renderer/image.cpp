//
// Copyright(C) 2014-2015 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//      RGB(A) image management
//

#include "png.h"
#include "kexlib.h"
#include "renderMain.h"

kexCvar cvarGamma("gl_gamma", CVF_FLOAT|CVF_CONFIG, "1", "TODO");

//
// ------------------------------------------------------
//
// common palette structure
//
// ------------------------------------------------------
//
typedef struct
{
    byte r;
    byte g;
    byte b;
    byte a;
} palette_t;

//
// ------------------------------------------------------
//
// tga structure
//
// ------------------------------------------------------
//

typedef struct
{
    byte infolen;
    byte has_cmap;
    byte type;
    short cmap_start;
    short cmap_len;
    byte cmap_bits;
    short xorigin;
    short yorigin;
    short width;
    short height;
    byte pixel_bits;
    byte flags;
} tgaheader_t;

enum tga_type
{
    TGA_TYPE_INDEXED        = 1,
    TGA_TYPE_RGB            = 2,
    TGA_TYPE_BW             = 3,
    TGA_TYPE_RLE_INDEXED    = 9,
    TGA_TYPE_RLE_RGB        = 10,
    TGA_TYPE_RLE_BW         = 11
};

//
// ------------------------------------------------------
//
// bmp structure
//
// ------------------------------------------------------
//

typedef struct
{
    char id[2];
    ulong fileSize;
    ulong u1;
    ulong dataOffset;
    ulong headerSize;
    ulong width;
    ulong height;
    word planes;
    word bits;
    ulong compression;
    ulong dataSize;
    ulong hRes;
    ulong vRes;
    ulong colors1;
    ulong colors2;
    palette_t palette[256];
} bmpheader_t;

//
// ------------------------------------------------------
//
// png local variables and callbacks
//
// ------------------------------------------------------
//

static byte *pngReadData;

//
// PNGRowSize
//

d_inline static size_t PNGRowSize(int width, byte bits)
{
    if(bits >= 8)
    {
        return ((width * bits) >> 3);
    }
    else
    {
        return (((width * bits) + 7) >> 3);
    }
}

//
// PNGReadFunc
//

static void PNGReadFunc(png_structp ctx, png_bytep area, png_size_t size)
{
    memcpy(area, pngReadData, size);
    pngReadData += size;
}

//
// ------------------------------------------------------
//
// image class
//
// ------------------------------------------------------
//

//
// kexImage::kexImage
//

kexImage::kexImage(void)
{
    this->data = NULL;
    this->width = 0;
    this->height = 0;
    this->origwidth = 0;
    this->origheight = 0;
    this->colorMode = TCR_RGBA;
}

//
// kexImage::kexImage
//

kexImage::kexImage(const int w, const int h, const texColorMode_t mode)
{
    this->width = w;
    this->height = h;
    this->origwidth = w;
    this->origheight = h;
    this->colorMode = mode;

    Alloc();
}

//
// kexImage::~kexImage
//

kexImage::~kexImage(void)
{
    if(data)
    {
        delete[] data;
    }
}

//
// kexImage::GetRGBGamma
//

byte kexImage::GetRGBGamma(int c)
{
    float f = cvarGamma.GetFloat();

    if(f == 1.0f)
    {
        return c;
    }

    return (byte)MIN(kexMath::Pow((float)c, (1.0f + (0.01f * f))), 255);
}

//
// kexImage::Alloc
//

void kexImage::Alloc(void)
{
    int size = width * height * (colorMode == TCR_RGBA ? 4 : 3);

    data = new byte[size];
    memset(data, 0, size);
}

//
// kexImage::LoadFromFile
//

void kexImage::LoadFromFile(const char *file)
{
    byte *fileData;

    strcpy(filePath, file);

    if(kex::cPakFiles->OpenFile(file, &fileData, hb_static) == 0)
    {
        return;
    }

    if(kexStr::IndexOf(file, ".tga\0") != -1)
    {
        LoadFromTGA(fileData);
    }
    else if(kexStr::IndexOf(file, ".bmp\0") != -1)
    {
        LoadFromBMP(fileData);
    }
    else if(kexStr::IndexOf(file, ".png\0") != -1)
    {
        LoadFromPNG(fileData);
    }
    else
    {
        data = NULL;
        kex::cSystem->Warning("kexImage::LoadFromFile(%s) - Unknown file format\n", file);
    }

    Mem_Free(fileData);
}

//
// kexImage::LoadFromScreenBuffer
//

void kexImage::LoadFromScreenBuffer(void)
{
    int pack;
    int col;

    origwidth   = kex::cSystem->VideoWidth();
    origheight  = kex::cSystem->VideoHeight();
    width       = origwidth;
    height      = origheight;
    col         = (width * 3);
    colorMode   = TCR_RGB;

    Alloc();

    dglGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    dglPixelStorei(GL_PACK_ALIGNMENT, 1);
    dglFlush();
    dglReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    dglPixelStorei(GL_PACK_ALIGNMENT, pack);
}

//
// kexImage::LoadFromFrameBuffer
//

void kexImage::LoadFromFrameBuffer(kexFBO &fbo)
{
    int pack;
    int col;

    origwidth   = fbo.Width();
    origheight  = fbo.Height();
    width       = origwidth;
    height      = origheight;
    col         = (width * 3);
    colorMode   = TCR_RGB;

    Alloc();

    fbo.Bind();

    dglGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    dglPixelStorei(GL_PACK_ALIGNMENT, 1);
    dglFlush();
    dglReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    dglPixelStorei(GL_PACK_ALIGNMENT, pack);

    fbo.UnBind();
}

//
// kexImage::LoadFromTGA
//

void kexImage::LoadFromTGA(byte *input)
{
    byte *tgafile;
    byte *rover;
    tgaheader_t tga;
    byte tmp[2];
    byte *data_r;
    int r;
    int c;
    palette_t *p;
    int bitStride = 0;

    tgafile = input;
    rover = tgafile;
    data = NULL;

    tga.infolen = *rover++;
    tga.has_cmap = *rover++;
    tga.type = *rover++;

    tmp[0]          = rover[0];
    tmp[1]          = rover[1];
    tga.cmap_start  = kex::cEndian->SwapLE16(*((short*)tmp));
    rover += 2;
    tmp[0]          = rover[0];
    tmp[1]          = rover[1];
    tga.cmap_len    = kex::cEndian->SwapLE16(*((short*)tmp));
    rover += 2;
    tga.cmap_bits   = *rover++;
    tga.xorigin     = kex::cEndian->SwapLE16(*((short*)rover));
    rover += 2;
    tga.yorigin     = kex::cEndian->SwapLE16(*((short*)rover));
    rover += 2;
    tga.width       = kex::cEndian->SwapLE16(*((short*)rover));
    rover += 2;
    tga.height      = kex::cEndian->SwapLE16(*((short*)rover));
    rover += 2;
    tga.pixel_bits  = *rover++;
    tga.flags       = *rover++;

    if(tga.infolen != 0)
    {
        rover += tga.infolen;
    }

    origwidth = tga.width;
    origheight = tga.height;
    width = kexMath::RoundPowerOfTwo(origwidth);
    height = kexMath::RoundPowerOfTwo(origheight);

    switch(tga.type)
    {
    case TGA_TYPE_INDEXED:
        switch(tga.pixel_bits)
        {
        case 8:
            p = (palette_t*)rover;

            switch(tga.cmap_bits)
            {
            case 24:
                colorMode = TCR_RGB;
                Alloc();
                rover += (3 * tga.cmap_len);

                for(r = tga.height-1; r >= 0; r--)
                {
                    data_r = data + r * width * 3;
                    for(c = 0; c < tga.width; c++)
                    {
                        *data_r++ = GetRGBGamma(p[*rover].b);
                        *data_r++ = GetRGBGamma(p[*rover].g);
                        *data_r++ = GetRGBGamma(p[*rover].r);
                        rover++;
                    }
                }
                break;
            case 32:
                colorMode = TCR_RGBA;
                Alloc();
                rover += (4 * tga.cmap_len);

                for(r = tga.height-1; r >= 0; r--)
                {
                    data_r = data + r * width * 4;
                    for(c = 0; c < tga.width; c++)
                    {
                        *data_r++ = GetRGBGamma(p[*rover].b);
                        *data_r++ = GetRGBGamma(p[*rover].g);
                        *data_r++ = GetRGBGamma(p[*rover].r);
                        *data_r++ = p[*rover].a;
                        rover++;
                    }
                }
                break;
            default:
                kex::cSystem->Error("%i-bit color map not supported for %s", tga.cmap_bits, filePath);
                break;
            }
            break;
        case 24:
            kex::cSystem->Error("24 bits (indexed) is not supported for %s", filePath);
        case 32:
            kex::cSystem->Error("32 bits (indexed) is not supported for %s", filePath);
            break;
        default:
            kex::cSystem->Error("unknown pixel bit for %s", filePath);
            break;
        }
        break;
    case TGA_TYPE_RGB:
        if(tga.pixel_bits == 32)
        {
            colorMode = TCR_RGBA;
            bitStride = 4;
        }
        else
        {
            colorMode = TCR_RGB;
            bitStride = 3;
        }

        Alloc();

        for(r = tga.height-1; r >= 0; r--)
        {
            data_r = data + r * width * bitStride;
            for(c = 0; c < tga.width; c++)
            {
                switch(tga.pixel_bits)
                {
                case 24:
                    *data_r++ = GetRGBGamma(rover[2]);
                    *data_r++ = GetRGBGamma(rover[1]);
                    *data_r++ = GetRGBGamma(rover[0]);
                    rover += 3;
                    break;
                case 32:
                    *data_r++ = GetRGBGamma(rover[2]);
                    *data_r++ = GetRGBGamma(rover[1]);
                    *data_r++ = GetRGBGamma(rover[0]);
                    *data_r++ = rover[3];
                    rover += 4;
                    break;
                default:
                    kex::cSystem->Error("unknown pixel bit for %s", filePath);
                    break;
                }
            }
        }
        break;
    case TGA_TYPE_BW:
        kex::cSystem->Error("grayscale images is not supported for %s", filePath);
        break;
    case TGA_TYPE_RLE_INDEXED:
        kex::cSystem->Error("RLE indexed images is not supported for %s", filePath);
        break;
    case TGA_TYPE_RLE_RGB:
        kex::cSystem->Error("RLE images is not supported for %s", filePath);
        break;
    case TGA_TYPE_RLE_BW:
        kex::cSystem->Error("RLE grayscale images is not supported for %s", filePath);
        break;
    default:
        kex::cSystem->Error("%s has unknown tga type", filePath);
        break;
    }
}

//
// kexImage::WriteTGA
//

void kexImage::WriteTGA(kexBinFile &binFile)
{
    tgaheader_t tga;
    int bits = colorMode == TCR_RGB ? 3 : 4;

    memset(&tga, 0, sizeof(tgaheader_t));

    tga.type = 2;
    tga.cmap_bits = 0;
    tga.has_cmap = 0;
    tga.cmap_len = 0;
    tga.pixel_bits = colorMode == TCR_RGB ? 24 : 32;
    tga.flags = 8;
    tga.width = origwidth;
    tga.height = origheight;

    binFile.Write8(tga.infolen);
    binFile.Write8(tga.has_cmap);
    binFile.Write8(tga.type);
    binFile.Write16(tga.cmap_start);
    binFile.Write16(tga.cmap_len);
    binFile.Write8(tga.cmap_bits);
    binFile.Write16(tga.yorigin);
    binFile.Write16(tga.xorigin);
    binFile.Write16(tga.width);
    binFile.Write16(tga.height);
    binFile.Write8(tga.pixel_bits);
    binFile.Write8(tga.flags);

    for(int i = 0; i < (tga.width * tga.height); i++)
    {
        binFile.Write8(data[i * bits + 2]);
        binFile.Write8(data[i * bits + 1]);
        binFile.Write8(data[i * bits + 0]);

        if(colorMode == TCR_RGBA)
        {
            binFile.Write8(data[i * bits + 4]);
        }
    }
}

//
// kexImage::LoadFromBMP
//

void kexImage::LoadFromBMP(byte *input)
{
    byte *rover = input;
    bmpheader_t bmp;

    bmp.id[0]       = *rover++;
    bmp.id[1]       = *rover++;
    bmp.fileSize    = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;
    bmp.u1          = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;
    bmp.dataOffset  = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;
    bmp.headerSize  = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;
    bmp.width       = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;
    bmp.height      = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;
    bmp.planes      = kex::cEndian->SwapLE16(*(short*)rover);
    rover += 2;
    bmp.bits        = kex::cEndian->SwapLE16(*(short*)rover);
    rover += 2;
    bmp.compression = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;
    bmp.dataSize    = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;
    bmp.hRes        = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;
    bmp.vRes        = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;
    bmp.colors1     = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;
    bmp.colors2     = kex::cEndian->SwapLE32(*(long*)rover);
    rover += 4;

    if(bmp.bits == 8)
    {
        memcpy(bmp.palette, rover, sizeof(palette_t) * 256);
        rover += (sizeof(palette_t) * 256);
    }

    if(bmp.id[0] != 'B' && bmp.id[1] != 'M')
    {
        kex::cSystem->Error("bitmap (%s) has unknown header ID ('BM' only supported\n", filePath);
    }
    if(bmp.compression != 0)
    {
        kex::cSystem->Error("compression not supported for bitmap (%s)\n", filePath);
    }
    if(bmp.bits < 8)
    {
        kex::cSystem->Error("monochrome and 4-bit pixels not supported for bitmap (%s)\n", filePath);
    }

    int bitStride = 0;

    int cols = kexMath::Abs(bmp.width);
    int rows = kexMath::Abs(bmp.height);

    origwidth = cols;
    origheight = rows;
    width = kexMath::RoundPowerOfTwo(origwidth);
    height = kexMath::RoundPowerOfTwo(origheight);

    if(bmp.bits != 32)
    {
        bitStride = 3;
        colorMode = TCR_RGB;
    }
    else
    {
        bitStride = 4;
        colorMode = TCR_RGBA;
    }

    Alloc();

    for(int y = rows-1; y >= 0; y--)
    {
        byte *buf = data + (y * width * bitStride);

        for(int x = 0; x < cols; x++)
        {
            byte rgba[4];
            word rgb16;
            int palIdx;

            switch(bmp.bits)
            {
            case 8:
                palIdx = *rover++;
                *buf++ = bmp.palette[palIdx].b;
                *buf++ = bmp.palette[palIdx].g;
                *buf++ = bmp.palette[palIdx].r;
                break;
            case 16:
                rgb16 = *(word*)buf;
                buf += 2;
                *buf++ = (rgb16 & (31 << 10)) >> 7;
                *buf++ = (rgb16 & (31 << 5)) >> 2;
                *buf++ = (rgb16 & 31) << 3;
                break;
            case 24:
                rgba[2] = *rover++;
                rgba[1] = *rover++;
                rgba[0] = *rover++;
                *buf++ = rgba[0];
                *buf++ = rgba[1];
                *buf++ = rgba[2];
                break;
            case 32:
                rgba[2] = *rover++;
                rgba[1] = *rover++;
                rgba[0] = *rover++;
                rgba[3] = *rover++;
                *buf++ = rgba[0];
                *buf++ = rgba[1];
                *buf++ = rgba[2];
                *buf++ = rgba[3];
                break;
            default:
                kex::cSystem->Error("bitmap (%s) has unknown pixel format (%i)\n", filePath, bmp.bits);
                break;
            }
        }
    }
}

//
// kexImage::LoadFromPNG
//

void kexImage::LoadFromPNG(byte *input)
{
    png_structp png_ptr;
    png_infop   info_ptr;
    png_uint_32 w;
    png_uint_32 h;
    int         bit_depth;
    int         color_type;
    int         interlace_type;
    int         pixel_depth;
    int         row;
    int         rowSize;
    byte        **row_pointers;

    // setup struct
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if(png_ptr == NULL)
    {
        kex::cSystem->Error("Failed to create png read struct");
        return;
    }

    // setup info struct
    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        kex::cSystem->Error("Failed to create png info struct");
        return;
    }

    pngReadData = input;

    // setup callback function for reading data
    png_set_read_fn(png_ptr, NULL, PNGReadFunc);

    // read png information
    png_read_info(png_ptr, info_ptr);

    // get IHDR chunk
    png_get_IHDR(png_ptr,
                 info_ptr,
                 &w,
                 &h,
                 &bit_depth,
                 &color_type,
                 &interlace_type,
                 NULL,
                 NULL);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        kex::cSystem->Error("Paletted PNGs is not yet supported");
        return;
    }

    // refresh png information
    png_read_update_info(png_ptr, info_ptr);

    // refresh data in IHDR chunk
    png_get_IHDR(png_ptr,
                 info_ptr,
                 &w,
                 &h,
                 &bit_depth,
                 &color_type,
                 &interlace_type,
                 NULL,
                 NULL);

    // get the size of each row
    pixel_depth = bit_depth;

    if(color_type == PNG_COLOR_TYPE_RGB)
    {
        pixel_depth *= 3;
        colorMode = TCR_RGB;
    }
    else if(color_type == PNG_COLOR_TYPE_RGB_ALPHA)
    {
        pixel_depth *= 4;
        colorMode = TCR_RGBA;
    }

    rowSize = PNGRowSize(w, pixel_depth);

    origwidth = (int)w;
    origheight = (int)h;

    // TODO: support texture padding for PNG images
    width = origwidth;
    height = origheight;

    Alloc();

    row_pointers = (byte**)Mem_AllocStatic(sizeof(byte*)*origheight);

    for(row = 0; row < origheight; row++)
    {
        row_pointers[row] = data + (row * rowSize);
    }

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, info_ptr);

    Mem_Free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}

//
// kexImage::WritePNG
//

void kexImage::WritePNG(kexBinFile &binFile)
{
    png_structp     png_ptr;
    png_infop       info_ptr;
    int             i = 0;

    // setup png pointer
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if(png_ptr == NULL)
    {
        kex::cSystem->Error("Failed creating png_ptr");
        return;
    }

    // setup info pointer
    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL)
    {
        png_destroy_write_struct(&png_ptr,  NULL);
        kex::cSystem->Error("Failed creating PNG info_ptr");
        return;
    }

    png_init_io(png_ptr, binFile.Handle());

    // setup image
    png_set_IHDR(
        png_ptr,
        info_ptr,
        width,
        height,
        8,
        colorMode == TCR_RGB ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_DEFAULT);

    // add png info to data
    png_write_info(png_ptr, info_ptr);

    for(i = height-1; i >= 0; --i)
    {
        png_write_row(png_ptr, data + i * (width * (colorMode == TCR_RGB ? 3 : 4)));
    }

    // cleanup
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
}

//
// kexImage::Blit
//

void kexImage::Blit(kexImage &image, const int x, const int y)
{
    const int bits = colorMode == TCR_RGB ? 3 : 4;
    const int srcBits = image.colorMode == TCR_RGB ? 3 : 4;
    const int srcW = image.Width();
    const int srcH = image.Height();

    for(int h = 0; h < srcH; h++)
    {
        byte *pdst = data;
        byte *psrc = image.data;

        for(int w = 0; w < srcW; w++)
        {
            int dstdelta = ((width * (h + y)) + (w + x)) * bits;
            int srcdelta = ((srcW * h) + w) * srcBits;

            pdst[dstdelta + 0] = psrc[srcdelta + 0];
            pdst[dstdelta + 1] = psrc[srcdelta + 1];
            pdst[dstdelta + 2] = psrc[srcdelta + 2];

            if(colorMode == TCR_RGBA)
            {
                if(image.colorMode == TCR_RGB)
                {
                    pdst[dstdelta + 3] = 0xff;
                }
                else
                {
                    if(psrc[srcdelta + 3])
                    {
                        pdst[dstdelta + 3] = psrc[srcdelta + 3];
                    }
                }
            }
        }
    }
}

//
// kexImage::Resample
//

void kexImage::Resample(const int newWidth, const int newHeight)
{
}

//
// kexImage::FlipVertical
//

void kexImage::FlipVertical(void)
{
    if(data == NULL)
    {
        return;
    }

    int bitStride   = (colorMode == TCR_RGBA) ? 4 : 3;
    byte *buffer    = new byte[(width * height) * bitStride];
    int col         = (width * bitStride);
    int offset1;
    int offset2;

    for(int i = 0; i < height / 2; i++)
    {
        for(int j = 0; j < col; j++)
        {
            offset1 = (i * col) + j;
            offset2 = ((height - (i + 1)) * col) + j;

            buffer[j] = data[offset1];
            data[offset1] = data[offset2];
            data[offset2] = buffer[j];
        }
    }

    delete[] buffer;
}
