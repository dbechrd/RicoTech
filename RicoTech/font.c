#include "font.h"
#include "const.h"
#include "util.h"
#include <GL/gl3w.h>
#include <stdlib.h>

#define BFG_RS_NONE  0x0      // Blend flags
#define BFG_RS_ALPHA 0x1
#define BFG_RS_RGB   0x2
#define BFG_RS_RGBA  0x4

#define BFG_MAXSTRING 255     // Maximum string length

#define WIDTH_DATA_OFFSET  20 // Offset to width data with BFF file
#define MAP_DATA_OFFSET   276 // Offset to texture image data with BFF file

#define RICO_FONT_POOL_SIZE 10
static struct font rico_font_pool[RICO_FONT_POOL_SIZE];
static uint32 next_uid = 1;

struct bff_header
{
    unsigned char ID1, ID2;
    unsigned char BPP;
    int ImageWidth, ImageHeight, CellWidth, CellHeight;
    unsigned char StartPoint;
};

struct font *make_font(const char *filename)
{
    struct font *font = NULL;
    char *buffer = NULL;
    long length;

    char bpp;
    int ImgX, ImgY;

    //TODO: Handle out-of-memory
    //TODO: Implement reuse of pool objects
    if (next_uid >= RICO_FONT_POOL_SIZE)
        return NULL;

    // Allocate font struct
    font = &rico_font_pool[next_uid++];

    // Read font file
    buffer = file_contents(filename, &length);
    if (!buffer) return NULL;

    // Check file signature
    if ((uchar)buffer[0] != 0xBF || (uchar)buffer[1] != 0xF2)
        goto cleanup;

    // Read bff header
    /*ImgX = swap_32bit((int)buffer[2]);
    ImgY = swap_32bit((int)buffer[6]);
    font->CellX = buffer[10];
    font->CellY = buffer[14];*/
    
    memcpy(&ImgX, &buffer[2], sizeof(int));
    memcpy(&ImgY, &buffer[6], sizeof(int));
    memcpy(&font->CellX, &buffer[10], sizeof(int));
    memcpy(&font->CellY, &buffer[14], sizeof(int));

    bpp = buffer[18];
    font->Base = buffer[19];

    // Check filesize
    if (length != (MAP_DATA_OFFSET + ((ImgX * ImgY) * (bpp / 8))))
        goto cleanup;

    // Calculate font params
    font->RowPitch = ImgX / font->CellX;
    font->ColFactor = (float)font->CellX / ImgX;
    font->RowFactor = (float)font->CellY / ImgY;
    font->YOffset = font->CellY;

    // Determine blending options based on BPP
    switch (bpp)
    {
    case 8: // Greyscale
        font->RenderStyle = BFG_RS_ALPHA;
        break;
    case 24: // RGB
        font->RenderStyle = BFG_RS_RGB;
        break;
    case 32: // RGBA
        font->RenderStyle = BFG_RS_RGBA;
        break;
    default: // Unsupported BPP
        goto cleanup;
    }

    // Store character widths
    memcpy(font->Width, &buffer[WIDTH_DATA_OFFSET], 256);

    // Create texture
    glGenTextures(1, &font->TexID);
    glBindTexture(GL_TEXTURE_2D, font->TexID);

    // Fonts should be rendered at native resolution, so no need for filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Stop chararcters from bleeding over edges
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Tex creation params are dependent on BPP
    switch (font->RenderStyle)
    {
    case BFG_RS_ALPHA:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ImgX, ImgY, 0, GL_RED,
                     GL_UNSIGNED_BYTE, &buffer[MAP_DATA_OFFSET]);
        break;

    case BFG_RS_RGB:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ImgX, ImgY, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, &buffer[MAP_DATA_OFFSET]);
        break;

    case BFG_RS_RGBA:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImgX, ImgY, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, &buffer[MAP_DATA_OFFSET]);
        break;
    }

cleanup:
    free(buffer);
    return font;
}

void font_render(const struct font *font, const char *text, struct col4 bg)
{

}

/*

bool Load(char *fname);
void SetScreen(int x, int y);
void SetCursor(int x, int y);
void SetColor(float Red, float Green, float Blue);
void ReverseYAxis(bool State);
void Select();
void Bind();
void SetBlend();
void Print(char *Text);
void Print(char *Text, int x, int y);
void ezPrint(char *Text, int x, int y);
int  GetWidth(char *Text);

int CellX,CellY,YOffset,RowPitch;
char Base;
char Width[256];
GLuint TexID;
int CurX,CurY;
float RowFactor,ColFactor;
int RenderStyle;
float Rd,Gr,Bl;
bool InvertYAxis;

 */