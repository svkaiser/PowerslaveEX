#include <stdio.h>
#include <io.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>

#include "brrezfile.h"

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned int rcolor;
typedef unsigned long big;
typedef byte* cache;

#define _PAD4(x)	x -= (4 - ((dword) x & 3)) & 3
#define _PAD8(x)	x -= (8 - ((dword) x & 7)) & 7
#define _PAD16(x)	x -= (16 - ((dword) x & 15)) & 15
#define _PAD32(x)	x -= (32 - ((dword) x & 31)) & 31
#define _PAD64(x)	x -= (64 - ((dword) x & 63)) & 63

#define _PAD2(x)   x += (2 + ((dword) x & 1)) & 1

#define _ZEDPAD(x)  (x + ((0x800 - (x & 0x7FF)) & 0x7FF))

#define Com_WriteMem8(p, x)             \
    *p = x; p++

#define Com_WriteMem16(p, x)            \
    Com_WriteMem8(p, x & 0xff);         \
    Com_WriteMem8(p, (x >> 8) & 0xff)

#define Com_WriteMem32(p, x)            \
    Com_WriteMem8(p, x & 0xff);         \
    Com_WriteMem8(p, (x >> 8) & 0xff);  \
    Com_WriteMem8(p, (x >> 16) & 0xff); \
    Com_WriteMem8(p, (x >> 24) & 0xff)

int ReadFile(const char *name, byte **buffer) {
    FILE *fp;

    errno = 0;

    if((fp = fopen(name, "rb"))) {
        size_t length;

        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *buffer = (byte*)malloc(length+1);
        memset(*buffer, 0, length+1);

        if(fread(*buffer, 1, length, fp) == length) {
            fclose(fp);
            return length;
        }
        
        free(*buffer);
        *buffer = NULL;
        fclose(fp);
   }
   
   return -1;
}

char *va(char *str, ...) {
    va_list v;
    static char vastr[1024];
	
    va_start(v, str);
    vsprintf(vastr, str,v);
    va_end(v);
    
    return vastr;	
}

typedef struct {
    byte r;
    byte g;
    byte b;
    byte a;
} dPalette_t;

typedef struct {
    byte infolen;
    byte has_cmap;
    byte type;
    short cmap_start;
    short cmap_len;
    byte cmap_bits;
    short yorigin;
    short xorigin;
    short width;
    short height;
    byte pixel_bits;
    byte flags;
} tgaheader_t;

#define NUMACTORS   182

// data is read in a linear fashion. size values determines offsets for
// the next set of data to read
//
// seems almost similar to Quake's BSP format but not quite
typedef struct {
    word u1;
    word texDataSize1;  // combine with texDataSize2 (must also be aligned with _ZEDPAD)
    word texDataSize2;
    word audioCount;
    int audioDataSize;  // must be aligned with _ZEDPAD
    int levelDataSize;  // must be aligned with _ZEDPAD
    word u3;
    word hullCount;
    word faceCount;
    word polyCount;
    word vertexCount;
    word uvCount;
    word thingCount;
    word eventCount;
    word spriteCount;
    word sprFrameCount;
    word sprInfoCount;
    word sprOffsetCount;
    short actorSpriteTable[NUMACTORS];
    short hudSpriteTable[10];
    short mipmapTranslation[16];
    byte glob[0x7E38];  // bunch of data pertaining to sprite/audio caching and mipmaps; don't care for now
} zedHeader_t;

// convex hulls
// at first I thought these were sectors
// determines visibility based on which hull you're standing in. checks for linked faces
// for visibility into the next hull. worse case scenario it walks through ALL hulls and linked faces
//
// also impossible to fall out of world. trying to walk into the void will clip all movement
typedef struct {
    word faceStart;
    word faceEnd; // for some reason you have to add '2' to account for ceilings and floors
    word lightLevel;
    short ceilingHeight;
    short floorHeight;
    word u3; // start ID for something
    word u4; // end ID for something
    short ceilingSlope;
    short floorSlope;
    short u5[6];
    word flags;
    short u6[26]; // always 0, filled in during level load
} hull_t;

// faces
typedef struct {
    short startID; // -1 if it links to another hull
    short endID; // -1 if it links to another hull
    word vertexStart; // seems to determine collision and view clipping. not sure how this is interpreted in game
    short hullID; // which hull to link to (aka like sidedefs in Doom)
    short angle;
    short nx;
    short ny;
    short nz;
    short u5; // appears to be some sort of flags value (0x1 for solid, 0x2000 for sidedef)
    short u6; // could be a tag ID
    word polyStartID;
    word polyEndID;
    short u7;
    short u8;
    short u9;
    short u10;
    short u11;
    short u12;
} face_t;

// polys
// each wall can have multiple polys to simulate tesselation which
// is also needed for smooth lighting effects
typedef struct {
    byte indices[4]; // vertex draw order
    short textureID;
    word uvID;
    short flipped; // either 1 or 0
    short u1;
} poly_t;

// vertices
typedef struct {
    short x;
    short y;
    short z;
    short light; // vertex lighting
} vertex_t;

// uv mapping
typedef struct {
    byte topLeft[2];
    byte topRight[2];
    byte bottomRight[2];
    byte bottomLeft[2];
} uvmap_t;

// things
//
// type list:
// 0    :player
// 12   :blue scorpian
// 83   :short fire pot
// 148  :team doll
typedef struct {
    short u1; // unknown. set to 25 for player
    short x;
    short params;
    short y;
    short tag;
    short z;
    short hullID;
    short type; // 0 for player
    short u4;
    short u5;
} thing_t;

typedef struct {
    short type;
    short sector;
    short tag;
    short u1;
} event_t;

typedef struct {
    byte u1;
    byte u2;
    byte u3;
    byte u4;
    short palette;
} textureLookup_t;

static dPalette_t pal[32][256];

typedef struct {
    byte *data;
    int p;
    int w;
    int h;
} textureCompare_t;

textureCompare_t g_textures[0x1000];
int g_numtextures = 0;

static word texturemapslot[24][200];

textureCompare_t g_spritetextures[0x4000];
int g_numsprites = 0;

int HasSprites(byte *data, const int w, const int h) {
    for(int i = 0; i < g_numsprites; ++i) {
        if(w != g_spritetextures[i].w || h != g_spritetextures[i].h) {
            continue;
        }

        if(memcmp(data, g_spritetextures[i].data, w * h) == 0) {
            return i;
        }
    }
    return -1;
}

static void ConvertPalette(dPalette_t* palette, word* data, int indexes) {
    int i;
    short val;
    
    for(i = 0; i < indexes; i++) {
        // Read the next packed short from the input buffer.
        val = data[i];
        
        // Unpack and expand to 8bpp, then flip from BGR to RGB.

        palette[i].r = (byte)(((val & 0x001F) << 3));
        palette[i].g = (byte)(((val & 0x03E0) >> 2));
        palette[i].b = (byte)(((val & 0x7C00) >> 7));

        if(val & 0x8000) {
            palette[i].a = 1;
        }
        else {
            palette[i].a = 0;
        }
    }

    // HACK HACK HACK
    if(palette == pal[1]) {
        palette[254].r = 156;
        palette[254].g = 74;
        palette[254].b = 32;
    }
}

#define MAXTEXSIZE	2048
#define MINTEXSIZE	1

static int PadDims(int n) {
    int mask = 1;
    
    while(mask < 0x40000000) {
        if(n == mask || (n & (mask-1)) == n)
            return mask;
        
        mask <<= 1;
    }
    return n;
}

void WriteTexture(byte *data, dPalette_t *pal, const int width, const int height, const char *fileName) {
    tgaheader_t tga;
    byte *texdata;
    int tgaSize;

    texdata = data;
    memset(&tga, 0, sizeof(tgaheader_t));

    /*
    tga.type = 1;
    tga.cmap_bits = 32;
    tga.has_cmap = 1;
    tga.cmap_len = 256;
    tga.pixel_bits = 8;
    tga.flags = 8;
    tga.width = width;
    tga.height = height;

    tgaSize = sizeof(tgaheader_t) + 1024 + (tga.width * tga.height);
    */
    tga.type = 2;
    tga.cmap_bits = 32;
    tga.has_cmap = 0;
    tga.cmap_len = 0;
    tga.pixel_bits = 32;
    tga.flags = 8;
    tga.width = width;
    tga.height = height;

    tgaSize = sizeof(tgaheader_t) + (tga.width * tga.height) * 4;

    byte *tgaData = (byte*)malloc(tgaSize);
    byte *rover = tgaData;

    Com_WriteMem8(rover, tga.infolen);
    Com_WriteMem8(rover, tga.has_cmap);
    Com_WriteMem8(rover, tga.type);
    Com_WriteMem16(rover, tga.cmap_start);
    Com_WriteMem16(rover, tga.cmap_len);
    Com_WriteMem8(rover, tga.cmap_bits);
    Com_WriteMem16(rover, tga.yorigin);
    Com_WriteMem16(rover, tga.xorigin);
    Com_WriteMem16(rover, tga.width);
    Com_WriteMem16(rover, tga.height);
    Com_WriteMem8(rover, tga.pixel_bits);
    Com_WriteMem8(rover, tga.flags);

    /*for(int col = 255; col >= 0; col--) {
        Com_WriteMem8(rover, pal[col].b);
        Com_WriteMem8(rover, pal[col].g);
        Com_WriteMem8(rover, pal[col].r);
        Com_WriteMem8(rover, pal[col].a);
    }*/

    for(int row = tga.height - 1; row >= 0; row--) {
        for(int col = 0; col < tga.width; col++) {
            int c = texdata[row * tga.width + col];

            Com_WriteMem8(rover, pal[c].b);
            Com_WriteMem8(rover, pal[c].g);
            Com_WriteMem8(rover, pal[c].r);
            Com_WriteMem8(rover, (c == 0xff) ? 0 : 0xff);
        }
    }

    FILE *f = fopen(fileName, "wb");
    fwrite(tgaData, tgaSize, 1, f);
    fclose(f);

    free(tgaData);
}

void WriteRawTexture(byte *data, const int width, const int height, const char *fileName) {
    tgaheader_t tga;
    byte *texdata;
    int tgaSize;

    texdata = data;
    memset(&tga, 0, sizeof(tgaheader_t));

    tga.type = 2;
    tga.cmap_bits = 24;
    tga.has_cmap = 0;
    tga.cmap_len = 0;
    tga.pixel_bits = 24;
    tga.flags = 8;
    tga.width = width;
    tga.height = height;

    tgaSize = sizeof(tgaheader_t) + (tga.width * tga.height) * 3;

    byte *tgaData = (byte*)malloc(tgaSize);
    byte *rover = tgaData;

    Com_WriteMem8(rover, tga.infolen);
    Com_WriteMem8(rover, tga.has_cmap);
    Com_WriteMem8(rover, tga.type);
    Com_WriteMem16(rover, tga.cmap_start);
    Com_WriteMem16(rover, tga.cmap_len);
    Com_WriteMem8(rover, tga.cmap_bits);
    Com_WriteMem16(rover, tga.yorigin);
    Com_WriteMem16(rover, tga.xorigin);
    Com_WriteMem16(rover, tga.width);
    Com_WriteMem16(rover, tga.height);
    Com_WriteMem8(rover, tga.pixel_bits);
    Com_WriteMem8(rover, tga.flags);

    for(int row = tga.height - 1; row >= 0; row--) {
        for(int col = 0; col < tga.width; col++) {
            Com_WriteMem8(rover, texdata[(row * tga.width + col) * 3 + 2]);
            Com_WriteMem8(rover, texdata[(row * tga.width + col) * 3 + 1]);
            Com_WriteMem8(rover, texdata[(row * tga.width + col) * 3 + 0]);
        }
    }

    FILE *f = fopen(fileName, "wb");
    fwrite(tgaData, tgaSize, 1, f);
    fclose(f);

    free(tgaData);
}

byte *GetSpriteData(byte *data, const int offset, const int mapid, const int index,
                    int *outwidth, int *outheight, int *palette) {
    word *spriteDims = (word*)(data + 0x5800);
    short *spriteHeight = (short*)(data + 0x6000);
    word *spritePalettes = (word*)(data + 0x6800);
    int *spriteOffs = (int*)(data + 0x7000);
    byte *sprites = (byte*)(&data[offset]);
    byte *spriteTexture;
    byte *spriteTextureRover;
    int total;
    byte *spr;
    byte *spriteRover;
    word dims;
    int sprSize;
    int sprHeight;

    spr = (sprites + spriteOffs[index]);
    spriteRover = spr;
    dims = spriteDims[index];

    total = 0;
    sprHeight = 0;
    sprSize = spriteOffs[index+1]-spriteOffs[index];

    if(spriteOffs[index+1] == 0) {
        sprSize = spriteOffs[index]-spriteOffs[index-1];
    }

    if(sprSize < 0) {
        return NULL;
    }

    if(sprSize == 0 || dims == 0) {
        return NULL;
    }

    if(outwidth) {
        *outwidth = dims;
    }

    if(spriteHeight[index] > 0) {
        sprHeight = spriteHeight[index];

        if(outheight) {
            *outheight = sprHeight;
        }
        spriteTexture = (byte*)malloc((dims*sprHeight) * 4);
        memcpy(spriteTexture, spr, (dims*sprHeight));
    }
    else {
        sprHeight = -(spriteHeight[index]);

        if(outheight) {
            *outheight = sprHeight;
        }

        spriteTexture = (byte*)calloc(1, (dims*sprHeight) * 4);
        spriteTextureRover = spriteTexture;

        while(total < (dims * sprHeight)) {
            int maskCount = *(spriteRover++);
            int pixelCount = *(spriteRover++);

            for(int j = 0; j < maskCount; j++) {
                *(spriteTextureRover++) = 255;
            }

            for(int j = 0; j < pixelCount; j++) {
                *(spriteTextureRover++) = *(spriteRover++);
            }

            total += (maskCount + pixelCount);
        }
    }

    int pl = spritePalettes[index] - 1;

    if(pl < 0) {
        pl = 1; // ???
    }

    if(palette) {
        *palette = pl;
    }

    WriteTexture(spriteTexture, pal[pl],
        dims, sprHeight, va("sprites/%02d/sprite%04d.tga", mapid, index));

    if(HasSprites(spriteTexture, dims, sprHeight) == -1)
    {
        g_spritetextures[g_numsprites].data = (byte*)malloc(dims * sprHeight);
        memcpy(g_spritetextures[g_numsprites].data, spriteTexture, dims * sprHeight);
        g_spritetextures[g_numsprites].p = pl;
        g_spritetextures[g_numsprites].w = dims;
        g_spritetextures[g_numsprites].h = sprHeight;

        WriteTexture(spriteTexture, pal[pl],
            dims, sprHeight, va("sprites/new/sprite%04d.tga", g_numsprites));

        g_numsprites++;
    }

    return spriteTexture;
}

short *GetSoundData(byte *data, unsigned int len, unsigned int *outsize) {
    const int f[5][2] = {
        { 0,    0   },
        { 60,   0   },
        { 115,  -52 },
        { 98,   -55 },
        { 122,  -60 }
    };

    short s_1 = 0, s_2 = 0;
    int numclips = 0;
    int nsamples = len / 16 * 28;
    short *pcm = (short*)malloc(sizeof(short) * nsamples);
    byte *pos;
    short *outpos = pcm;

    for(pos = data; pos < data + len; pos += 16) {
        int weight = pos[0] >> 4;
        int shift = pos[0] & 0xf;

        for(int j = 2; j < 16; j++) {
            int samples = pos[j] & 0xf;

            for(int k = 0; k < 2; k++) {
                samples <<= 12;

                if(samples & 0x8000) {
                    samples |= 0xffff0000;
                }

                samples >>= shift;
                samples += s_1 * f[weight][0] >> 6;
                samples += s_2 * f[weight][1] >> 6;

                if(samples > 0x7fff) {
                    samples = 0x7fff;
                    numclips++;
                }
                else if(samples < -0x7fff) {
                    samples = -0x7fff;
                    numclips++;
                }

                s_2 = s_1;
                s_1 = (short)samples;
                *outpos++ = s_1;

                samples = pos[j] >> 4;
            }
        }
    }

    *outsize = sizeof(short) * nsamples;
    return pcm;
}

typedef struct {
    byte channel[2];
} xaChannelBlock_t;

typedef struct {
    byte parameters[16];
    byte data[112];
} xaData_t;

typedef struct {
    byte header1[12];
    byte minute;
    byte second;
    byte sectors;
    byte mode;
    byte fileNum;
    byte channel;
    byte subMode;
    byte info;
    int dup;
    xaData_t data[18];
    byte pad[24];
} xaHeader_t;

char Signed4bit(char number) {
    if((number & 0x8) == 0x8) {
        return(char)((number & 0x7)-8);
    }
    else {
        return (char)number;
    }
}

int MinMax(int number, int min, int max) {
    if(number < min) {
        return min;
    }
    if(number > max) {
        return max;
    }

    return number;
}

void DecodeBlock(int &cnt, short *out, FILE *f, xaData_t *xa, byte block, byte nibble, int dst, int &old, int &older) {
    const int pos_xa_adpcm_table[5] = { 0, 60, 115, 98,  122 };
    const int neg_xa_adpcm_table[5] = { 0, 0, -52, -55, -60 };

    byte shift = (xa->parameters[4+block*2+nibble] & 0xf);
    byte filter = xa->parameters[4+block*2+nibble] >> 4;

    int f0 = pos_xa_adpcm_table[filter];
    int f1 = neg_xa_adpcm_table[filter];

    for(int j = 0; j < 28; j++) {
        int samples = (xa->data[block + j * 4] >> (nibble * 4)) & 0xf;

        samples <<= 12;

        if(samples & 0x8000) {
            samples |= 0xffff0000;
        }

        //samples >>= 12;

        samples >>= shift;
        samples += old * f0 >> 6;
        samples += older * f1 >> 6;

        if(samples > 0x7fff) {
            samples = 0x7fff;
        }
        else if(samples < -0x7fff) {
            samples = -0x7fff;
        }

        older = old;
        old = (short)samples;
        out[dst] = old;
        cnt++;
        dst += 2;
    }
}

int HasTexture(byte *data, const int w, const int h) {
    for(int i = 0; i < g_numtextures; ++i) {
        if(w != g_textures[i].w || h != g_textures[i].h) {
            continue;
        }

        if(memcmp(data, g_textures[i].data, w * h) == 0) {
            return i;
        }
    }
    return -1;
}

#include "filenames.h"

#if 1
int main(int argc, char **argv) {

    /*byte *data;
    byte *sprdata;
    int len = ReadFile("E:/Sources/Disassembly Projects/PowerSlave_PSX/MAP.XED", &data);
    sprdata = data + 1624;
    dPalette_t p[256];

    for(int fff = 0; fff < 35; ++fff) {
        if(*(short*)(sprdata) == 0) {
            sprdata += 2;
        }

        short w = *(short*)(sprdata + 0);
        short h = *(short*)(sprdata + 2);
        short s = *(short*)(sprdata + 4);
        int total = 0;

        sprdata += 6;

        byte *spriteTexture = (byte*)calloc(1, (w*-h) * 8);
        byte *spriteTextureRover = spriteTexture;

        byte *tmp = sprdata;

        while(total < (w * -h)) {
            int maskCount = *(tmp++);
            int pixelCount = *(tmp++);

            for(int j = 0; j < maskCount; j++) {
                *(spriteTextureRover++) = 255;
            }

            for(int j = 0; j < pixelCount; j++) {
                *(spriteTextureRover++) = *(tmp++);
            }

            total += (maskCount + pixelCount);
        }

        sprdata += s;
        ConvertPalette(p, (word*)sprdata, 256);

        sprdata += 512;

        WriteTexture(spriteTexture, p,
            w, -h, va("sprites/overworld_%02d.tga", fff));

        free(spriteTexture);
    }

    free(data);
    return 0;*/

    /*byte *mapdata;
    byte scratch[128*128*3];

    ReadFile("E:/Sources/Disassembly Projects/PowerSlave_PSX/MAP.OWT", &mapdata);

    short *tmpdata = (short*)mapdata;
    int map = 0;

    while(*tmpdata != -1) {
        int w = *tmpdata;
        tmpdata++;
        int h = *tmpdata;
        tmpdata++;

        byte *rover = scratch;

        for(int i = 0; i < (w * h); i++) {
            short val = *tmpdata;

            *rover++ = (byte)(((val & 0x001F) << 3) + 7);
            *rover++ = (byte)(((val & 0x03E0) >> 2) + 7);
            *rover++ = (byte)(((val & 0x7C00) >> 7) + 7);

            tmpdata++;
        }

        WriteRawTexture(scratch, w, h, va("OVERWORLD_%02d.tga", map++));
    }

    return 0;*/
    
    byte *xadata;

    int len = ReadFile("E:/Sources/Disassembly Projects/PowerSlave_PSX/RAMSES/RA1314.XA", &xadata);
    int blocks = len / sizeof(xaHeader_t);
    xaHeader_t *xa = (xaHeader_t*)xadata;

    FILE *wf = fopen("ra14.wav", "wb");
    unsigned int wavtemp;
    int freq = 18900;
    short tmpbuf[0x1000];
    int totalSize = 0;
    int bytesProcessed = 0;
    const int channel = 1;

    for(int i = 0; i < blocks; ++i) {
        xaHeader_t *xablock = &xa[i];

        if(xablock->channel != channel) {
            continue;
        }

        if(!(xablock->subMode & 0x4)) {
            continue;
        }

        totalSize += (112 * 18) * 2;
    }

    if(!(xa[0].info & 2)) {
        freq *= 2;
    }

    fwrite ("RIFF", 1, 4, wf);
    wavtemp = (totalSize / 16 * 28) * 2 + 36;
    fwrite (&wavtemp, 1, 4, wf);
    fwrite ("WAVE", 1, 4, wf);
    fwrite ("fmt ", 1, 4, wf);
    wavtemp = 16;
    fwrite (&wavtemp, 1, 4, wf);
    wavtemp = 1;
    fwrite (&wavtemp, 1, 2, wf);
    wavtemp = 1;
    fwrite (&wavtemp, 1, 2, wf);
    wavtemp = freq;
    fwrite (&wavtemp, 1, 4, wf);
    wavtemp = 2 * freq * 16;
    fwrite (&wavtemp, 1, 4, wf);
    wavtemp = 4;
    fwrite (&wavtemp, 1, 2, wf);
    wavtemp = 16;
    fwrite (&wavtemp, 1, 2, wf);

    fwrite ("data", 1, 4, wf);
    wavtemp = (totalSize / 16 * 28) * 2;
    fwrite (&wavtemp, 1, 4, wf);

    int dstLeft = 0, oldLeft = 0, olderLeft = 0, dstRight = 1, oldRight = 0, olderRight = 0;
    int cutoffSize = (totalSize*2)-4096;

    for(int i = 0; i < blocks; ++i) {
        xaHeader_t *xablock = &xa[i];

        if(xablock->channel != channel) {
            continue;
        }

        if(!(xablock->subMode & 0x4)) {
            continue;
        }

        if(bytesProcessed >= cutoffSize) {
            break;
        }

        for(int j = 0; j < 18; ++j) {
            xaData_t *xad = &xablock->data[j];
            int blk = 0;

            if(bytesProcessed >= cutoffSize) {
                break;
            }

            memset(tmpbuf, 0, sizeof(short) * 0x1000);

            for(blk = 0; blk < 4; blk++) {
                int cnt = 0;

                DecodeBlock(cnt, tmpbuf, wf, xad, blk, 0, dstLeft, oldLeft, olderLeft);
                DecodeBlock(cnt, tmpbuf, wf, xad, blk, 1, dstRight, oldRight, olderRight);

                fwrite(tmpbuf, 1, sizeof(short) * cnt, wf);
                bytesProcessed += (sizeof(short) * cnt);
            }
        }
    }

    fclose(wf);
    //fclose(xafile);
    free(xadata);
    return 0;
    

    unsigned int offset;
    int texDataSize;
    byte *data1;
    int *polyIndices;
    short *polyUV;
    zedHeader_t *header;
    short *mipmaptranslation;
    int spriteDataOffset = 0;
    FILE *f;

    for(int lx = 0; lx < sizeof(levelfiles) / sizeof(char*); lx++) {
        char *filepath;
        FILE *outmap;

        if(levelfiles[lx] == NULL) {
            break;
        }

        printf("reading %s/%s\n", argv[1], levelfiles[lx]);

        filepath = va("%s/%s.ZED", argv[1], levelfiles[lx]);

        if(ReadFile(filepath, &data1) == -1)
            return 1;

        header = (zedHeader_t*)data1;
        texDataSize = (header->texDataSize2 << 16) | header->texDataSize1;
        offset = sizeof(zedHeader_t) +
            _ZEDPAD(texDataSize) +
            _ZEDPAD(header->audioDataSize);

        mipmaptranslation = header->mipmapTranslation;

        //
        // SECTORS
        //
        f = fopen(va("%s_sectors.txt", levelfiles[lx]), "w");
        hull_t *chunk1 = (hull_t*)(&data1[offset]);

        for(int i = 0; i < header->hullCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%5i ", chunk1[i].faceStart);
            fprintf(f, "%5i ", chunk1[i].faceEnd);
            fprintf(f, "%5i ", chunk1[i].lightLevel);
            fprintf(f, "%5i ", chunk1[i].ceilingHeight);
            fprintf(f, "%5i ", chunk1[i].floorHeight);
            fprintf(f, "%12f ", chunk1[i].ceilingSlope / 11.38f);
            fprintf(f, "%12f ", chunk1[i].floorSlope / 11.38f);
            for(int j = 0; j < 6; j++) {
                fprintf(f, "%5i ", chunk1[i].u5[j]);
            }
            fprintf(f, "%5i ", chunk1[i].flags);
            for(int j = 0; j < 26; j++) {
                fprintf(f, "%5i ", chunk1[i].u6[j]);
            }
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(hull_t) * header->hullCount);

        //
        // LINES
        //
        f = fopen(va("%s_faces.txt", levelfiles[lx]), "w");
        face_t *chunk2 = (face_t*)(&data1[offset]);

        for(int i = 0; i < header->faceCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i ", chunk2[i].startID);
            fprintf(f, "%6i ", chunk2[i].endID);
            fprintf(f, "%6i ", chunk2[i].vertexStart);
            fprintf(f, "%6i ", chunk2[i].hullID);
            fprintf(f, "%12f ", chunk2[i].angle / 11.38f);
            fprintf(f, "%12f ", chunk2[i].nx / 16384.0f);
            fprintf(f, "%12f ", chunk2[i].ny / 16384.0f);
            fprintf(f, "%12f ", chunk2[i].nz / 16384.0f);
            fprintf(f, "%6i ", chunk2[i].u5);
            fprintf(f, "%6i ", chunk2[i].u6);
            fprintf(f, "%6i ", chunk2[i].polyStartID);
            fprintf(f, "%6i ", chunk2[i].polyEndID);
            fprintf(f, "%6i ", chunk2[i].u7);
            fprintf(f, "%6i ", chunk2[i].u8);
            fprintf(f, "%6i ", chunk2[i].u9);
            fprintf(f, "%6i ", chunk2[i].u10);
            fprintf(f, "%6i ", chunk2[i].u11);
            fprintf(f, "%6i ", chunk2[i].u12);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(face_t) * header->faceCount);

        //
        // POLYS
        //
        f = fopen(va("%s_polys.txt", levelfiles[lx]), "w");
        poly_t *chunk3 = (poly_t*)(&data1[offset]);

        for(int i = 0; i < header->polyCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i %6i %6i %6i ",
                chunk3[i].indices[0],
                chunk3[i].indices[1],
                chunk3[i].indices[2],
                chunk3[i].indices[3]);
            fprintf(f, "%6i ", chunk3[i].textureID);
            fprintf(f, "%6i ", chunk3[i].uvID);
            fprintf(f, "%6i ", chunk3[i].flipped);
            fprintf(f, "%6i ", chunk3[i].u1);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(poly_t) * header->polyCount);

        //
        // VERTICES
        //
        f = fopen(va("%s_vertices.txt", levelfiles[lx]), "w");
        vertex_t *chunk4 = (vertex_t*)(&data1[offset]);

        for(int i = 0; i < header->vertexCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i ", chunk4[i].x);
            fprintf(f, "%6i ", chunk4[i].y);
            fprintf(f, "%6i ", chunk4[i].z);
            fprintf(f, "%6i ", chunk4[i].light);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(vertex_t) * header->vertexCount);

        //
        // UV MAPPING
        //
        f = fopen(va("%s_mappings.txt", levelfiles[lx]), "w");
        uvmap_t *chunk5 = (uvmap_t*)(&data1[offset]);

        for(int i = 0; i < header->uvCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i/%i ", chunk5[i].topLeft[0], chunk5[i].topLeft[1]);
            fprintf(f, "%6i/%i ", chunk5[i].topRight[0], chunk5[i].topRight[1]);
            fprintf(f, "%6i/%i ", chunk5[i].bottomRight[0], chunk5[i].bottomRight[1]);
            fprintf(f, "%6i/%i ", chunk5[i].bottomLeft[0], chunk5[i].bottomLeft[1]);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(uvmap_t) * header->uvCount);

        //
        // THINGS
        //
        f = fopen(va("%s_actors.txt", levelfiles[lx]), "w");
        thing_t *chunk6 = (thing_t*)(&data1[offset]);

        for(int i = 0; i < header->thingCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i ", chunk6[i].u1);
            fprintf(f, "%6i ", chunk6[i].x);
            fprintf(f, "%6i ", chunk6[i].params);
            fprintf(f, "%6i ", chunk6[i].y);
            fprintf(f, "%6i ", chunk6[i].tag);
            fprintf(f, "%6i ", chunk6[i].z);
            fprintf(f, "%6i ", chunk6[i].hullID);
            fprintf(f, "%6i ", chunk6[i].type);
            fprintf(f, "%6i ", chunk6[i].u4);
            fprintf(f, "%6i ", chunk6[i].u5);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(thing_t) * header->thingCount);

        //
        // EVENTS
        //
        f = fopen(va("%s_events.txt", levelfiles[lx]), "w");
        event_t *chunk7 = (event_t*)(&data1[offset]);

        for(int i = 0; i < header->eventCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i ", chunk7[i].type);
            fprintf(f, "%6i ", chunk7[i].sector);
            fprintf(f, "%6i ", chunk7[i].tag);
            fprintf(f, "%6i ", chunk7[i].u1);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(event_t) * header->eventCount);

        f = fopen(va("%s.obj", levelfiles[lx]), "w");

        fprintf(f, va("mtllib %s_mat.mtl\n", levelfiles[lx]));

        for(int i = 0; i < header->vertexCount; i++) {
            float x = (float)chunk4[i].x / 256.0f;
            float y = (float)chunk4[i].z / 256.0f;
            float z = (float)chunk4[i].y / 256.0f;
            fprintf(f, "v %f %f %f\n", x, y, -z);
        }

        for(int i = 0; i < header->uvCount; i++) {
            fprintf(f, "vt %f %f\n",
                (float)chunk5[i].topLeft[0] / 63.0f,
                1.0f - (float)chunk5[i].topLeft[1] / 63.0f);

            fprintf(f, "vt %f %f\n",
                (float)chunk5[i].topRight[0] / 63.0f,
                1.0f - (float)chunk5[i].topRight[1] / 63.0f);

            fprintf(f, "vt %f %f\n",
                (float)chunk5[i].bottomRight[0] / 63.0f,
                1.0f - (float)chunk5[i].bottomRight[1] / 63.0f);

            fprintf(f, "vt %f %f\n",
                (float)chunk5[i].bottomLeft[0] / 63.0f,
                1.0f - (float)chunk5[i].bottomLeft[1] / 63.0f);
        }

        /*
        fprintf(f, "o collision\n");
        for(int k = 0; k < header->hullCount; k++) {
            int start = chunk1[k].faceStart;
            int end = chunk1[k].faceEnd;

            for(int i = start; i <= end; i++) {
                if(chunk2[i].startID == -1 || chunk2[i].endID == -1)
                    continue;

                int vstart = chunk2[i].vertexStart;
                int vend = chunk2[i+1].polyStartID;

                if(vend == 0)
                    vend = chunk2[i+1].vertexStart;

                if(vend - vstart <= 2)
                    continue;

                fprintf(f, "f ");
                for(int j = (vend-1); j >= vstart; j--) {
                    fprintf(f, "%i ", j+1);
                }
                fprintf(f, "\n");
            }

            int count = chunk2[(end+1)+1].polyStartID;
            if(count == 0)
                count = chunk2[(end+1)+1].vertexStart;

            count = count - chunk2[(end+1)].vertexStart;

            if(count <= 2)
                continue;

            for(int i = end+1; i < end+3; i++) {
                int vstart = chunk2[i].vertexStart;

                fprintf(f, "f ");
                for(int j = (vstart + count)-1; j >= vstart; j--) {
                    fprintf(f, "%i ", j+1);
                }
                fprintf(f, "\n");
            }
        }
        */

        word *palData = (word*)(data1 + 0x1800);

        for(int j = 0; j < 32; j++) {
            ConvertPalette(pal[j], &palData[256*j], 256);

            FILE *pf = fopen(va("palettes/pal%04d.act", j), "wb");
            for(int i = 0; i < 256; i++) {
                fwrite(&pal[j][i].r, 1, 1, pf);
                fwrite(&pal[j][i].g, 1, 1, pf);
                fwrite(&pal[j][i].b, 1, 1, pf);
            }

            fclose(pf);
        }

        WriteTexture(data1 + 0x8000, pal[0], 256, 256, va("textures/sky%02d.tga", lx));
        WriteTexture(data1 + 0x18000, pal[0], 64, 64, va("textures/sky%02d_01.tga", lx));
        WriteTexture(data1 + 0x19000, pal[0], 64, 64, va("textures/sky%02d_02.tga", lx));
        WriteTexture(data1 + 0x1A000, pal[0], 64, 64, va("textures/sky%02d_03.tga", lx));
        WriteTexture(data1 + 0x1B000, pal[0], 64, 64, va("textures/sky%02d_04.tga", lx));
        WriteTexture(data1 + 0x1C000, pal[1], 256, 64, "textures/hud.tga");

        byte *textureData = (data1 + 0x20000);
        textureLookup_t *lookup = (textureLookup_t*)(data1 + 586);

        FILE *f_mtl = fopen(va("%s_mat.mtl", levelfiles[lx]), "w");

        int scratch = 0;

#define writeshortasint(x) \
        scratch = x;    \
        fwrite(&scratch, 1, 4, outmap)

        outmap = fopen(va("%s.MAP", levelfiles[lx]), "wb");
        writeshortasint(104);
        writeshortasint(header->vertexCount);
        writeshortasint(header->hullCount);
        writeshortasint(header->faceCount);
        writeshortasint(header->polyCount);
        writeshortasint(header->uvCount);
        writeshortasint(header->eventCount);
        writeshortasint(header->thingCount);

#undef writeshortasint

        short skytexnamelen = (short)strlen(skyfiles[lx]);
        fwrite(&skytexnamelen, 1, 2, outmap);
        fwrite(skyfiles[lx], 1, skytexnamelen, outmap);

        int totaltextures = 0;
        int globaltexturelookup[104];

        memset(globaltexturelookup, -1, sizeof(int) * 104);

        for(int i = 0; i < 104; i++) {
            if(lookup[i].u2 == 0 || lookup[i].u3 == 0) {
                short len = 0;
                fwrite(&len, 1, 2, outmap);
                fwrite(&len, 1, 2, outmap);
                continue;
            }

            int w = lookup[i].u2+1;
            int h = lookup[i].u3+1;

            if(w == 63) {
                w = 64;
            }

            if(w == 31) {
                w = 32;
            }

            int pl = lookup[i].palette - 1;

            if(pl < 0) {
                pl = 1; // ???
            }

            int lookup = 0;

            //WriteTexture(textureData, pal[pl], w, h, va("textures/%02d/texture%03d.tga", lx, i));

            if((lookup = HasTexture(textureData, w, h)) == -1) {
                g_textures[g_numtextures].data = (byte*)malloc(w * h);
                memcpy(g_textures[g_numtextures].data, textureData, w * h);
                g_textures[g_numtextures].w = w;
                g_textures[g_numtextures].h = h;

                //WriteTexture(textureData, pal[pl], w, h, va("textures/new/texture%03d.tga",
                    //g_numtextures));

                texturemapslot[lx][i] = g_numtextures;
                lookup = g_numtextures;

                g_numtextures++;
            }

            textureData += (w * h);

            fprintf(f_mtl, "newmtl mat_%03d\n", i);
            fprintf(f_mtl, "Ns 96.078431\n");
            fprintf(f_mtl, "Ka 1.000 1.000 1.000\n");
            fprintf(f_mtl, "Kd 1.000 1.000 1.000\n");
            fprintf(f_mtl, "Ks 0.500000 0.500000 0.500000\n");
            fprintf(f_mtl, "Ni 1.000000\n");
            fprintf(f_mtl, "d 1.000000\n");

            for(int xxx = 0; texturenames[xxx] != NULL; xxx++) {
                int len = strlen(texturenames[xxx]);
                char id[4];

                len -= 7;
                id[0] = texturenames[xxx][len+0];
                id[1] = texturenames[xxx][len+1];
                id[2] = texturenames[xxx][len+2];
                id[3] = 0;

                int iid = atoi(id);

                if(iid == lookup) {
                    fprintf(f_mtl, "map_Kd textures/new/%s\n\n", texturenames2[xxx]);
                    globaltexturelookup[i] = xxx;
                    totaltextures++;

                    short len = strlen(texturenames[xxx]);
                    fwrite(&len, 1, 2, outmap);
                    fwrite(texturenames[xxx], 1, len, outmap);
                    break;
                }
            }

            //fprintf(f_mtl, "map_Kd textures/new/texture%03d.tga\n\n", lookup);

            //fprintf(f_mtl, "map_Kd textures\\%02d\\texture%03d.tga\n\n", lx, i);
            //fprintf(f_mtl, "map_Kd default.tga\n\n");
        }

        fclose(f_mtl);

        polyIndices = (int*)malloc((sizeof(int) * header->polyCount) * 4);
        polyUV = (short*)malloc((sizeof(short) * header->polyCount) * 4);

        memset(polyIndices, -1, (sizeof(int) * header->polyCount) * 4);
        memset(polyUV, -1, (sizeof(short) * header->polyCount) * 4);

        for(int k = 0; k < header->hullCount; k++) {
        //for(int k = 0; k < 1; k++) {
            int start = chunk1[k].faceStart;
            int end = chunk1[k].faceEnd;
            fprintf(f, "o sector_%03d\n", k);
            for(int i = start; i < end+3; i++) {
                if(chunk2[i].startID == -1 || chunk2[i].endID == -1)
                    continue;

                for(int j = chunk2[i].startID; j <= chunk2[i].endID; j++) {
                    int indices[4] = { 0, 0, 0, 0 };
                    int tcoords[4] = { 0, 0, 0, 0 };
                    int curIdx = 0;
                    float nx1 = (float)chunk2[i].nx / 16384.0f;
                    float ny1 = (float)chunk2[i].ny / 16384.0f;
                    float nz1 = (float)chunk2[i].nz / 16384.0f;
                    int texture = chunk3[j].textureID;

                    if(texture >= 88) {
                        texture = mipmaptranslation[texture-88];
                    }

                    fprintf(f, "usemtl mat_%03d\n", texture);

                    for(int idx = 0; idx < 4; idx++) {
                        if(chunk3[j].indices[idx] == chunk3[j].indices[(idx+1)&3])
                            continue;

                        indices[curIdx] = chunk2[i].polyStartID + chunk3[j].indices[idx] + 1;
                        tcoords[curIdx] = (chunk3[j].uvID * 4 + idx)+1;

                        curIdx++;
                    }

                    if(curIdx <= 2)
                        continue;

                    if(chunk3[j].flipped == 0) {
                        int x1 = chunk4[indices[0]-1].x;
                        int x2 = chunk4[indices[1]-1].x;
                        int x3 = chunk4[indices[2]-1].x;

                        int y1 = chunk4[indices[0]-1].y;
                        int y2 = chunk4[indices[1]-1].y;
                        int y3 = chunk4[indices[2]-1].y;

                        int z1 = chunk4[indices[0]-1].z;
                        int z2 = chunk4[indices[1]-1].z;
                        int z3 = chunk4[indices[2]-1].z;

                        float p1x = (float)(x2 - x1);
                        float p1y = (float)(y2 - y1);
                        float p1z = (float)(z2 - z1);

                        float p2x = (float)(x3 - x2);
                        float p2y = (float)(y3 - y2);
                        float p2z = (float)(z3 - z2);

                        float cx = p2z * p1y - p1z * p2y;
                        float cy = p2x * p1z - p1x * p2z;
                        float cz = p1x * p2y - p2x * p1y;

                        float d = sqrtf(cx*cx+cy*cy+cz*cz);

                        if(d != 0) {
                            float nx2, ny2, nz2;

                            nx2 = cx / d;
                            ny2 = cy / d;
                            nz2 = cz / d;

                            if(nx2 * nx1 + ny2 * ny1 + nz2 * nz1 > 0) {
                                chunk3[j].flipped ^= 1;
                            }
                        }
                    }

                    fprintf(f, "f ");

                    if(chunk3[j].flipped == 0) {
                        for(int idx = curIdx-1, in = 0; idx >= 0; idx--, in++) {
                            fprintf(f, "%i/%i ", indices[idx], tcoords[idx]);
                            polyIndices[j * 4 + in] = (indices[idx]-1) - chunk2[i].polyStartID;
                            polyUV[j * 4 + in] = tcoords[idx]-1;
                        }
                    }
                    else {
                        for(int idx = 0, in = 0; idx < curIdx; idx++, in++) {
                            fprintf(f, "%i/%i ", indices[idx], tcoords[idx]);
                            polyIndices[j * 4 + in] = (indices[idx]-1) - chunk2[i].polyStartID;
                            polyUV[j * 4 + in] = tcoords[idx]-1;
                        }
                    }
                    fprintf(f, " \n");
                }
            }
        }

        fclose(f);

#if 1
        for(int i = 0; i < header->vertexCount; i++) {
            short l = chunk4[i].light << 1;

            if(l > 255) {
                l = 255;
            }

            byte fl = (byte)l;
            byte tp = 0xff;

            fwrite(&chunk4[i].x, 1, 2, outmap);
            fwrite(&chunk4[i].y, 1, 2, outmap);
            fwrite(&chunk4[i].z, 1, 2, outmap);
            fwrite(&fl, 1, 1, outmap);
            fwrite(&fl, 1, 1, outmap);
            fwrite(&fl, 1, 1, outmap);
            fwrite(&tp, 1, 1, outmap);
        }

        for(int i = 0; i < header->hullCount; i++) {
            float tmp;
            short tmp2;

            tmp2 = chunk1[i].lightLevel;

            if(tmp2 > 255) {
                tmp2 = 255;
            }

            fwrite(&chunk1[i].faceStart, 1, 2, outmap);
            fwrite(&chunk1[i].faceEnd, 1, 2, outmap);
            fwrite(&tmp2, 1, 2, outmap);
            fwrite(&chunk1[i].ceilingHeight, 1, 2, outmap);
            fwrite(&chunk1[i].floorHeight, 1, 2, outmap);

            tmp = chunk1[i].ceilingSlope / 11.38f;
            fwrite(&tmp, 1, 4, outmap);

            tmp = chunk1[i].floorSlope / 11.38f;
            fwrite(&tmp, 1, 4, outmap);

            if(lx == 8) {
                if(chunk2[chunk1[i].faceEnd+1].startID == -1 ||
                    chunk2[chunk1[i].faceEnd+1].endID == -1) {
                        chunk1[i].flags |= 16;
                }
            }

            fwrite(&chunk1[i].flags, 1, 2, outmap);
        }

        for(int i = 0; i < header->faceCount; i++) {
            float tmp;

            fwrite(&chunk2[i].startID, 1, 2, outmap);
            fwrite(&chunk2[i].endID, 1, 2, outmap);
            fwrite(&chunk2[i].vertexStart, 1, 2, outmap);
            fwrite(&chunk2[i].hullID, 1, 2, outmap);

            tmp = chunk2[i].angle / 11.38f;
            fwrite(&tmp, 1, 4, outmap);

            tmp = chunk2[i].nx / 16384.0f;
            fwrite(&tmp, 1, 4, outmap);
            tmp = chunk2[i].ny / 16384.0f;
            fwrite(&tmp, 1, 4, outmap);
            tmp = chunk2[i].nz / 16384.0f;
            fwrite(&tmp, 1, 4, outmap);

            fwrite(&chunk2[i].u5, 1, 2, outmap);
            fwrite(&chunk2[i].u6, 1, 2, outmap);
            fwrite(&chunk2[i].polyStartID, 1, 2, outmap);
            fwrite(&chunk2[i].polyEndID, 1, 2, outmap);
        }

        for(int i = 0; i < header->polyCount; i++) {
            int texture = chunk3[i].textureID;

            if(texture >= 88) {
                texture = mipmaptranslation[texture-88];
            }

            fwrite(&polyIndices[i * 4 + 0], 1, 1, outmap);
            fwrite(&polyIndices[i * 4 + 1], 1, 1, outmap);
            fwrite(&polyIndices[i * 4 + 2], 1, 1, outmap);
            fwrite(&polyIndices[i * 4 + 3], 1, 1, outmap);
            fwrite(&polyUV[i * 4 + 0], 1, 2, outmap);
            fwrite(&polyUV[i * 4 + 1], 1, 2, outmap);
            fwrite(&polyUV[i * 4 + 2], 1, 2, outmap);
            fwrite(&polyUV[i * 4 + 3], 1, 2, outmap);
            fwrite(&texture, 1, 2, outmap);
            fwrite(&chunk3[i].flipped, 1, 2, outmap);
        }

        free(polyIndices);
        free(polyUV);

        polyIndices = NULL;
        polyUV = NULL;

        for(int i = 0; i < header->uvCount; i++) {
            float tl1 = (float)chunk5[i].topLeft[0] / 63.0f;
            float tl2 = 1.0f - (float)chunk5[i].topLeft[1] / 63.0f;
            float tr1 = (float)chunk5[i].topRight[0] / 63.0f;
            float tr2 = 1.0f - (float)chunk5[i].topRight[1] / 63.0f;
            float br1 = (float)chunk5[i].bottomRight[0] / 63.0f;
            float br2 = 1.0f - (float)chunk5[i].bottomRight[1] / 63.0f;
            float bl1 = (float)chunk5[i].bottomLeft[0] / 63.0f;
            float bl2 = 1.0f - (float)chunk5[i].bottomLeft[1] / 63.0f;

            fwrite(&tl1, 1, 4, outmap);
            fwrite(&tl2, 1, 4, outmap);
            fwrite(&tr1, 1, 4, outmap);
            fwrite(&tr2, 1, 4, outmap);
            fwrite(&br1, 1, 4, outmap);
            fwrite(&br2, 1, 4, outmap);
            fwrite(&bl1, 1, 4, outmap);
            fwrite(&bl2, 1, 4, outmap);
        }

        for(int i = 0; i < header->eventCount; i++) {
            fwrite(&chunk7[i].type, 1, 2, outmap);
            fwrite(&chunk7[i].sector, 1, 2, outmap);
            fwrite(&chunk7[i].tag, 1, 2, outmap);
            fwrite(&chunk7[i].u1, 1, 2, outmap);
        }

        for(int i = 0; i < header->thingCount; i++) {
            float tmp;

            fwrite(&chunk6[i].type, 1, 2, outmap);
            fwrite(&chunk6[i].hullID, 1, 2, outmap);
            fwrite(&chunk6[i].x, 1, 2, outmap);
            fwrite(&chunk6[i].y, 1, 2, outmap);
            fwrite(&chunk6[i].z, 1, 2, outmap);
            fwrite(&chunk6[i].tag, 1, 2, outmap);
            fwrite(&chunk6[i].params, 1, 2, outmap);
            fwrite(&chunk6[i].u1, 1, 2, outmap);

            tmp = (float)chunk6[i].u4 / 11.38f;
            fwrite(&tmp, 1, 4, outmap);
        }

        fclose(outmap);
#endif

#if 1
        
        word *spriteDims = (word*)(data1 + 0x5800);
        short *spriteHeight = (short*)(data1 + 0x6000);
        word *spritePalettes = (word*)(data1 + 0x6800);
        f = fopen(va("%s_spriteDims.txt", levelfiles[lx]), "w");
        for(int i = 0; i < header->spriteCount; i++) {
            fprintf(f, "%i: %i %i\n", i, spriteDims[i], abs(spriteHeight[i]));
        }
        fclose(f);

        int *spriteOffs = (int*)(data1 + 0x7000);
        byte *sprites = (byte*)(&data1[offset]);
        spriteDataOffset = offset;

        f = fopen(va("%s_spriteOffs.txt", levelfiles[lx]), "w");
        for(int i = 0; i < header->spriteCount; i++) {
            fprintf(f, "%i: %i\n", i, spriteOffs[i]);
        }
        fclose(f);

        int lastspr = header->spriteCount-1;
        offset += spriteOffs[lastspr];

        if(spriteHeight[lastspr] > 0) {
            offset += (spriteDims[lastspr] * spriteHeight[lastspr]);
        }
        else {
            int total = 0;
            word dims = spriteDims[lastspr];
            int sprHeight = -spriteHeight[lastspr];
            byte *spriteRover = (sprites + spriteOffs[lastspr]);

            while(total < (dims * sprHeight)) {
                int maskCount = *(spriteRover++);
                int pixelCount = *(spriteRover++);

                offset += 2;

                for(int j = 0; j < pixelCount; j++) {
                    spriteRover++;
                    offset++;
                }

                total += (maskCount + pixelCount);
            }
        }

        byte *fuck = (byte*)(&data1[offset]);

        while(*((short*)(fuck)) != 0) {
            fuck++;
            offset++;
        }

        for(int i = 0; i < header->spriteCount; i++) {
            GetSpriteData(data1, spriteDataOffset, lx, i, NULL, NULL, NULL);
        }

        //
        // SPRITE FRAMES
        //
        f = fopen(va("%s_sprframes.txt", levelfiles[lx]), "w");
        short *chunk8 = (short*)(&data1[offset]);

        for(int i = 0; i < header->sprFrameCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i / %i", chunk8[i], chunk8[i+1] - chunk8[i]);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(short) * (header->sprFrameCount+1));

        //
        // SPRITE INFO
        //
        f = fopen(va("%s_sprinfo.txt", levelfiles[lx]), "w");
        short *chunk9 = (short*)(&data1[offset]);

        for(int i = 0; i < header->sprInfoCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i / %i", chunk9[i], chunk9[i+1] - chunk9[i]);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(short) * (header->sprInfoCount+1));

        //
        // SPRITE FLAGS
        //
        f = fopen(va("%s_sprflags.txt", levelfiles[lx]), "w");
        short *chunk10 = (short*)(&data1[offset]);

        for(int i = 0; i < header->sprInfoCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i", chunk10[i]);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(short) * header->sprInfoCount);

        //
        // SPRITE SOUNDS
        //
        f = fopen(va("%s_sprsounds.txt", levelfiles[lx]), "w");
        short *chunk11 = (short*)(&data1[offset]);

        for(int i = 0; i < header->sprInfoCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i", chunk11[i]);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(short) * header->sprInfoCount);

        //
        // SPRITE OFFSETS
        //
        f = fopen(va("%s_sproffsets.txt", levelfiles[lx]), "w");
        short *chunk12a = (short*)(&data1[offset]);
        short *chunk12b = (short*)(&data1[offset + (sizeof(short) * header->sprOffsetCount)]);

        for(int i = 0; i < header->sprOffsetCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i ", chunk12a[i]);
            fprintf(f, "%6i ", chunk12b[i]);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += ((sizeof(short) * 2) * header->sprOffsetCount);

        //
        // SPRITE ID
        //
        f = fopen(va("%s_sprframeid.txt", levelfiles[lx]), "w");
        short *chunk13 = (short*)(&data1[offset]);

        for(int i = 0; i < header->sprOffsetCount; i++) {
            fprintf(f, "##%03d:", i);
            fprintf(f, "%6i", chunk13[i]);
            fprintf(f, "\n");
        }
        fclose(f);
        offset += (sizeof(short) * header->sprOffsetCount);

        short *chunk14 = (short*)(&data1[offset]);
        offset += (sizeof(short) * header->sprOffsetCount);

        f = fopen(va("%s_spriteanims.txt", levelfiles[lx]), "w");
        for(int i = 0; i < header->sprFrameCount; i++) {
            int frames = chunk8[i+1] - chunk8[i];

            fprintf(f, "spriteanim_%03d\n{\n", i);

            fprintf(f, "    // frames: %4i\n", frames);
            for(int j = 0; j < frames; j++) {
                int infoidx = chunk8[i] + j;
                int numgroups = chunk9[infoidx+1] - chunk9[infoidx];
                int frameinfo = chunk9[infoidx];

                fprintf(f, "    frame\n    {\n");
                if(numgroups <= 0) {
                    fprintf(f, "        // ##%03d %4i %4i (no groups)\n", j,
                        chunk11[infoidx], chunk10[infoidx]);
                    fprintf(f, "        delay 1\n");
                }
                else {
                    fprintf(f, "        // ##%03d %4i %4i\n", j, chunk11[infoidx], chunk10[infoidx]);
                    fprintf(f, "        sprites\n");
                    fprintf(f, "        {\n");
                    for(int k = 0; k < numgroups; k++) {
                        int idx = frameinfo + k;

                        fprintf(f, "            { \"spr:%i\", 0, %6i, %4i, %4i }\n",
                            chunk13[idx],
                            chunk12a[idx],
                            chunk12b[idx],
                            chunk14[idx]);
                    }
                    fprintf(f, "        }\n");
                }

                fprintf(f, "\n    }\n\n");
            }

            fprintf(f, "}\n\n");
        }
        fclose(f);

#if 1
        if(lx == 5) {
            static const char *rotnames[8] = {
                "rotation_N", "rotation_NE", "rotation_E", "rotation_ES",
                "rotation_S", "rotation_SW", "rotation_W", "rotation_WN"
            };
            f = fopen("anubis_temp.txt", "w");

            for(int i = 82; i < 147; i++) {
                int frames = chunk8[i+1] - chunk8[i];

                fprintf(f, "spriteanim_%03d\n{\n", i);

                fprintf(f, "    // frames: %4i\n", frames);
                for(int j = 0; j < frames; j++) {
                    fprintf(f, "    frame\n    {\n");

                    for(int jj = 0; jj < 8; jj++) {

                        int infoidx = chunk8[i+jj] + j;
                        int numgroups = chunk9[infoidx+1] - chunk9[infoidx];
                        int frameinfo = chunk9[infoidx];

                        fprintf(f, "        // ##%03d %4i %4i\n", j, chunk11[infoidx], chunk10[infoidx]);
                        fprintf(f, "        %s\n", rotnames[jj]);
                        fprintf(f, "        {\n");
                        fprintf(f, "            sprites\n");
                        fprintf(f, "            {\n");
                        for(int k = 0; k < numgroups; k++) {
                            int idx = frameinfo + k;

                            fprintf(f, "                { \"monsters/queen\", %i, %6i, %4i, %4i }\n",
                                chunk13[idx] - 261,
                                chunk12a[idx],
                                chunk12b[idx],
                                chunk14[idx]);
                        }
                        fprintf(f, "            }\n");
                        fprintf(f, "        }\n");
                    }

                    fprintf(f, "\n    }\n\n");
                }

                fprintf(f, "}\n\n");
            }
            fclose(f);
        }


#endif

#endif

#if 0
        word *soundSizes = (word*)(data1 + 0x4BA);
        int *soundRate = (int*)(data1 + 0x1000);
        byte *audioData = &data1[(sizeof(zedHeader_t) + _ZEDPAD(texDataSize))];
        unsigned int wavsize;

        for(int i = 0; i < header->audioCount; i++) {
            byte *wavdata = (byte*)GetSoundData(audioData, soundSizes[i], &wavsize);
            f = fopen(va("sounds/%s_sfx%02d.wav", levelfiles[lx], i), "wb");
            unsigned int wavtemp;
            int freq = (int)((float)soundRate[i] * 10.74f);

            fwrite ("RIFF", 1, 4, f);
            wavtemp = (soundSizes[i] / 16 * 28) * 2 + 36;
            fwrite (&wavtemp, 1, 4, f);
            fwrite ("WAVE", 1, 4, f);
            fwrite ("fmt ", 1, 4, f);
            wavtemp = 16;
            fwrite (&wavtemp, 1, 4, f);
            wavtemp = 1;
            fwrite (&wavtemp, 1, 2, f);
            wavtemp = 1;
            fwrite (&wavtemp, 1, 2, f);
            wavtemp = freq;
            fwrite (&wavtemp, 1, 4, f);
            wavtemp = 2 * freq;
            fwrite (&wavtemp, 1, 4, f);
            wavtemp = 2;
            fwrite (&wavtemp, 1, 2, f);
            wavtemp = 16;
            fwrite (&wavtemp, 1, 2, f);

            fwrite ("data", 1, 4, f);
            wavtemp = (soundSizes[i] / 16 * 28) * 2;
            fwrite (&wavtemp, 1, 4, f);

            fwrite(wavdata, 1, wavsize, f);
            fclose(f);

            audioData += soundSizes[i];
        }
#endif

        free(data1);
    }

	return 0;
}

#else

typedef struct {
    unsigned int offset;
    unsigned int size;
} dentry_t;

typedef struct {
    unsigned int  version;      // Model version, must be 0x17 (23).
    dentry_t entities;          // List of Entities.
    dentry_t planes;            // Map Planes.
                                // numplanes = size/sizeof(plane_t)
    dentry_t miptex;            // Wall Textures.
    dentry_t vertices;          // Map Vertices.
                                // numvertices = size/sizeof(vertex_t)
    dentry_t visilist;          // Leaves Visibility lists.
    dentry_t nodes;             // BSP Nodes.
                                // numnodes = size/sizeof(node_t)
    dentry_t texinfo;           // Texture Info for faces.
                                // numtexinfo = size/sizeof(texinfo_t)
    dentry_t faces;             // Faces of each surface.
                                // numfaces = size/sizeof(face_t)
    dentry_t lightmaps;         // Wall Light Maps.
    dentry_t clipnodes;         // clip nodes, for Models.
                                // numclips = size/sizeof(clipnode_t)
    dentry_t leaves;            // BSP Leaves.
                                // numlaves = size/sizeof(leaf_t)
    dentry_t lface;             // List of Faces.
    dentry_t edges;             // Edges of faces.
                                // numedges = Size/sizeof(edge_t)
    dentry_t ledges;            // List of Edges.
    dentry_t models;            // List of Models.
                                // nummodels = Size/sizeof(model_t)
} dheader_t;

typedef struct {
    float x;
    float y;
    float z;
} qvtx_t;

typedef struct {
    word vtx1;
    word vtx2;
} qedge_t;

typedef struct {
    word plane_id;              // The plane in which the face lies
                                //           must be in [0,numplanes[ 
    word side;                  // 0 if in front of the plane, 1 if behind the plane
    unsigned int ledge_id;      // first edge in the List of edges
                                //           must be in [0,numledges[
    word ledge_num;             // number of edges in the List of edges
    word texinfo_id;            // index of the Texture info the face is part of
                                //           must be in [0,numtexinfos[ 
    byte typelight;             // type of lighting, for the face
    byte baselight;             // from 0xFF (dark) to 0 (bright)
    byte light[2];              // two additional light models  
    unsigned int lightmap;      // Pointer inside the general light map, or -1
                                // this define the start of the face light map
} qface_t;

typedef struct {
    float bound[2][3];          // The bounding box of the Model
    float origin[3];            // origin of model, usually (0,0,0)
    unsigned int node_id0;      // index of first BSP node
    unsigned int node_id1;      // index of the first Clip node
    unsigned int node_id2;      // index of the second Clip node
    unsigned int node_id3;      // usually zero
    unsigned int numleafs;      // number of BSP leaves
    unsigned int face_id;       // index of Faces
    unsigned int face_num;      // number of Faces
} qmodel_t;

typedef struct {
    dword type;                 // Special type of leaf
    dword vislist;              // Beginning of visibility lists
                                //     must be -1 or in [0,numvislist[
    short bound[2][3];          // Bounding box of the leaf
    word lface_id;              // First item of the list of faces
                                //     must be in [0,numlfaces[
    word lface_num;             // Number of faces in the leaf  
    byte sndwater;              // level of the four ambient sounds:
    byte sndsky;                //   0    is no sound
    byte sndslime;              //   0xFF is maximum volume
    byte sndlava;               //
} qleaf_t;

int main(int argc, char **argv) {
    byte *data1;
    dheader_t *header;
    FILE *f;

    if(ReadFile("start.bsp", &data1) == -1)
        return 1;

    header = (dheader_t*)data1;

    qvtx_t *vertices    = (qvtx_t*)(&data1[header->vertices.offset]);
    qedge_t *edges      = (qedge_t*)(&data1[header->edges.offset]);
    qface_t *faces      = (qface_t*)(&data1[header->faces.offset]);
    qmodel_t *models    = (qmodel_t*)(&data1[header->models.offset]);
    qleaf_t *leafs      = (qleaf_t*)(&data1[header->leaves.offset]);
    int *edgeList       = (int*)(&data1[header->ledges.offset]);
    short *faceList     = (short*)(&data1[header->lface.offset]);

    f = fopen("qvertices.txt", "w");
    for(dword i = 0; i < header->vertices.size / sizeof(qvtx_t); i++) {
        fprintf(f, "##%03d:", i);
        fprintf(f, "%12f ", vertices[i].x);
        fprintf(f, "%12f ", vertices[i].y);
        fprintf(f, "%12f ", vertices[i].z);
        fprintf(f, "\n");
    }
    fclose(f);

    f = fopen("qedges.txt", "w");
    for(dword i = 0; i < header->edges.size / sizeof(qedge_t); i++) {
        fprintf(f, "##%03d:", i);
        fprintf(f, "%6i ", edges[i].vtx1);
        fprintf(f, "%6i ", edges[i].vtx2);
        fprintf(f, "\n");
    }
    fclose(f);

    f = fopen("qfaces.txt", "w");
    for(dword i = 0; i < header->faces.size / sizeof(qface_t); i++) {
        fprintf(f, "##%03d:", i);
        fprintf(f, "%6i ", faces[i].plane_id);
        fprintf(f, "%6i ", faces[i].side);
        fprintf(f, "%6i ", faces[i].ledge_id);
        fprintf(f, "%6i ", faces[i].ledge_num);
        fprintf(f, "\n");
    }
    fclose(f);

    f = fopen("qmodels.txt", "w");
    for(dword i = 0; i < header->models.size / sizeof(qmodel_t); i++) {
        fprintf(f, "##%03d:", i);
        fprintf(f, "%6i ", models[i].face_id);
        fprintf(f, "%6i ", models[i].face_num);
        fprintf(f, "\n");
    }
    fclose(f);

    f = fopen("qleafs.txt", "w");
    for(dword i = 0; i < header->leaves.size / sizeof(qleaf_t); i++) {
        fprintf(f, "##%03d:", i);
        fprintf(f, "%6i ", leafs[i].lface_id);
        fprintf(f, "%6i ", leafs[i].lface_num);
        fprintf(f, "\n");
    }
    fclose(f);

    f = fopen("qedgeList.txt", "w");
    for(dword i = 0; i < header->ledges.size / sizeof(int); i++) {
        fprintf(f, "##%03d:", i);
        fprintf(f, "%i", edgeList[i]);
        fprintf(f, "\n");
    }
    fclose(f);

    f = fopen("qfaceList.txt", "w");
    for(dword i = 0; i < header->lface.size / sizeof(short); i++) {
        fprintf(f, "##%03d:", i);
        fprintf(f, "%i", faceList[i]);
        fprintf(f, "\n");
    }
    fclose(f);

    f = fopen("quake.obj", "w");

   for(dword i = 0; i < header->vertices.size / sizeof(qvtx_t); i++) {
        fprintf(f, "v %f %f %f\n",
            vertices[i].x / 256.0f,
            vertices[i].z / 256.0f,
            -vertices[i].y / 256.0f);
    }

   for(dword i = 0; i < header->leaves.size / sizeof(qleaf_t); i++) {
       qleaf_t *leaf = &leafs[i];

       if(leaf->lface_num == 0)
           continue;

       fprintf(f, "o leaf_%03d\n", i);

       for(dword j = 0; j < leaf->lface_num; j++) {
           qface_t *face = &faces[faceList[leaf->lface_id + j]];

           fprintf(f, "f ");
           for(dword k = 0; k < face->ledge_num; k++) {
               int lindex = edgeList[face->ledge_id + k];
               qedge_t *edge;

               if(lindex > 0) {
                   edge = &edges[lindex];
                   fprintf(f, "%i ", edge->vtx1+1);
               }
               else {
                   edge = &edges[-lindex];
                   fprintf(f, "%i ", edge->vtx2+1);
               }
           }

           fprintf(f, "\n");
       }
   }

   fclose(f);

    free(data1);

    return 0;
}

#endif

