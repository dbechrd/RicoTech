#include "rico_font.h"
#include "const.h"
#include "util.h"
#include "rico_mesh.h"
#include "rico_texture.h"
#include <GL/gl3w.h>
#include <stdlib.h>

#define BFG_RS_NONE  0x0  // Blend flags
#define BFG_RS_ALPHA 0x1
#define BFG_RS_RGB   0x2
#define BFG_RS_RGBA  0x4

#define BFG_MAXSTRING 255  // Maximum string length

#define WIDTH_DATA_OFFSET  20  // Offset to width data with BFF file
#define MAP_DATA_OFFSET   276  // Offset to texture image data with BFF file

struct bff_header
{
    unsigned char ID1, ID2;
    unsigned char BPP;
    int ImageWidth, ImageHeight, CellWidth, CellHeight;
    unsigned char StartPoint;
};

u32 RICO_FONT_DEFAULT = 0;
static struct rico_pool *fonts;

int rico_font_init(u32 pool_size)
{
    fonts = calloc(1, sizeof(*fonts));
    return pool_init("Fonts", pool_size, sizeof(struct rico_font), 0, fonts);
}

int font_init(const char *filename, u32 *_handle)
{
    enum rico_error err;
    *_handle = RICO_FONT_DEFAULT;

    #ifdef RICO_DEBUG_INFO
        printf("[font][init] filename=%s\n", filename);
    #endif

    err = pool_handle_alloc(fonts, _handle);
    if (err) return err;

    struct rico_font *font = pool_read(fonts, *_handle);
    uid_init(&font->uid, RICO_UID_FONT, filename, false);
	font->InvertYAxis = false;

    // Read font file
    char *buffer = NULL;
    int length;
    err = file_contents(filename, &length, &buffer);
    if (err) goto cleanup;

    // Check file signature
    if ((u8)buffer[0] != 0xBF || (u8)buffer[1] != 0xF2)
        goto cleanup;

    // Read bff header
    int width, height, bpp;
    memcpy(&width, &buffer[2], sizeof(int));
    memcpy(&height, &buffer[6], sizeof(int));
    memcpy(&font->CellX, &buffer[10], sizeof(int));
    memcpy(&font->CellY, &buffer[14], sizeof(int));

    bpp = buffer[18];
    font->Base = buffer[19];

    // Check filesize
    if (length != (MAP_DATA_OFFSET + (width * height * bpp / 8)))
        goto cleanup;

    // Calculate font params
    font->RowPitch = width / font->CellX;
    font->ColFactor = (float)font->CellX / width;
    font->RowFactor = (float)font->CellY / height;
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

    err = texture_load_pixels(filename, GL_TEXTURE_2D, width, height, bpp,
                              &buffer[MAP_DATA_OFFSET], &font->texture);
    if (err) goto cleanup;

    texture_request(font->texture);

cleanup:
    free(buffer);
    return err;
}

void font_free(u32 handle)
{
    struct rico_font *font = pool_read(fonts, handle);
    texture_free(font->texture);

    font->uid.uid = UID_NULL;
    pool_handle_free(fonts, handle);
}

static void font_setblend(const struct rico_font *font)
{
    // TODO: Preserve blend settings before changing
	switch(font->RenderStyle)
	{
	case BFG_RS_ALPHA:
        glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
		glEnable(GL_BLEND);
        break;
    case BFG_RS_RGB:
        glDisable(GL_BLEND);
        break;
    case BFG_RS_RGBA:
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		break;
	}
}

int font_render(u32 handle, int x, int y, struct col4 bg, const char *text,
                const char *mesh_name, enum rico_mesh_type type, u32 *_mesh,
                u32 *_texture)
{
    enum rico_error err;

    struct rico_font *font = pool_read(fonts, handle);

    //font_setblend(font);

    int text_len = strlen(text);

    // Truncate strings that are too long to render
    if (BFG_MAXSTRING < text_len)
        text_len = BFG_MAXSTRING;

    // TODO: We're wasting a lot of vertices here, use a proper triangle strip
    // Each letter is a quad
    int vertex_count = text_len * 4;
    // Each quad is two triangles, or 6 vertices
    int element_count = text_len * 6;

    struct mesh_vertex *vertices = calloc(vertex_count, sizeof(*vertices));
    GLuint *elements = calloc(element_count, sizeof(*elements));

    int idx_vertex = 0;
    int idx_element = 0;

    int cur_x = x;
    int cur_y = y - font->YOffset;

    int row, col;
    GLfloat u0, v0, u1, v1;
    int xOffset;
    for (int i = 0; i < text_len; i++)
    {
        if (text[i] == '\n') {
            cur_y -= font->YOffset;
            cur_x = x;
            continue;
        }

        row = (text[i] - font->Base) / font->RowPitch;
        col = (text[i] - font->Base) - (row * font->RowPitch);

        u0 = col * font->ColFactor;
        v0 = row * font->RowFactor;
        u1 = u0 + font->ColFactor;
        v1 = v0 + font->RowFactor;

        xOffset = font->Width[(int)text[i]];
        //xOffset = font->CellX;

        // Vertices for this character's quad
        vertices[idx_vertex++] = (struct mesh_vertex) {
            (struct vec3) {
                cur_x / 64.0f,
                cur_y / 64.0f,
                0.0f
            },
            bg,
            (struct vec3) { 1.0f, 1.0f, 1.0f },
            (struct tex2) { u0, v1 }
        };
        vertices[idx_vertex++] = (struct mesh_vertex) {
            (struct vec3) {
                (cur_x + xOffset) / 64.0f,
                cur_y / 64.0f,
                0.0f
            },
            bg,
            (struct vec3) { 1.0f, 1.0f, 1.0f },
            (struct tex2) { u1, v1 }
        };
        vertices[idx_vertex++] = (struct mesh_vertex) {
            (struct vec3) {
                (cur_x + xOffset) / 64.0f,
                (cur_y + font->YOffset) / 64.0f,
                0.0f
            },
            bg,
            (struct vec3) { 1.0f, 1.0f, 1.0f },
            (struct tex2) { u1, v0 }
        };
        vertices[idx_vertex++] = (struct mesh_vertex) {
            (struct vec3) {
                cur_x / 64.0f,
                (cur_y + font->YOffset) / 64.0f,
                0.0f
            },
            bg,
            (struct vec3) { 1.0f, 1.0f, 1.0f },
            (struct tex2) { u0, v0 }
        };

        //3  2
        //0  1

        //idx_vertex-1  idx_vertex-2
        //idx_vertex-4  idx_vertex-3

        // Triangles using this character's vertices
        elements[idx_element++] = idx_vertex - 4;
        elements[idx_element++] = idx_vertex - 3;
        elements[idx_element++] = idx_vertex - 2;

        elements[idx_element++] = idx_vertex - 4;
        elements[idx_element++] = idx_vertex - 2;
        elements[idx_element++] = idx_vertex - 1;

        cur_x += xOffset;
    }

    err = mesh_load(_mesh, mesh_name, type, vertex_count, vertices,
                    element_count, elements, GL_STATIC_DRAW);
    if (err) goto cleanup;

    *_texture = font->texture;

cleanup:
    free(vertices);
    free(elements);
    return err;
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