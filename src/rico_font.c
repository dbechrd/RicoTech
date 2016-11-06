#include "rico_font.h"
#include "const.h"
#include "util.h"
#include "rico_mesh.h"
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
static struct rico_font rico_font_pool[RICO_FONT_POOL_SIZE];
static uint32 next_uid = 1;

struct bff_header
{
    unsigned char ID1, ID2;
    unsigned char BPP;
    int ImageWidth, ImageHeight, CellWidth, CellHeight;
    unsigned char StartPoint;
};

struct rico_font *make_font(const char *filename)
{
	// TODO: Clean up malloc
    struct rico_font *font = calloc(1, sizeof(*font));
    char *buffer = NULL;
    int length;

	font->InvertYAxis = false;

    // TODO: Handle out-of-memory
    // TODO: Implement reuse of pool objects
    if (next_uid >= RICO_FONT_POOL_SIZE)
        return NULL;

    // Allocate font struct
    font = &rico_font_pool[next_uid++];

    // Read font file
    buffer = file_contents(filename, &length);
    if (!buffer)
		return NULL;

    // Check file signature
    if ((uchar)buffer[0] != 0xBF || (uchar)buffer[1] != 0xF2)
        goto cleanup;

    // Read bff header
    /*ImgX = swap_32bit((int)buffer[2]);
    ImgY = swap_32bit((int)buffer[6]);
    font->CellX = buffer[10];
    font->CellY = buffer[14];*/
    
    struct rico_texture *tex = calloc(1, sizeof(*tex));
    tex->target = GL_TEXTURE_2D;

    memcpy(&tex->width, &buffer[2], sizeof(int));
    memcpy(&tex->height, &buffer[6], sizeof(int));
    memcpy(&font->CellX, &buffer[10], sizeof(int));
    memcpy(&font->CellY, &buffer[14], sizeof(int));

    tex->bpp = buffer[18];
    font->Base = buffer[19];

    // Check filesize
    if (length != (MAP_DATA_OFFSET + (tex->width * tex->height * tex->bpp / 8)))
        goto cleanup;

    // Calculate font params
    font->RowPitch = tex->width / font->CellX;
    font->ColFactor = (float)font->CellX / tex->width;
    font->RowFactor = (float)font->CellY / tex->height;
    font->YOffset = font->CellY;

    // Determine blending options based on BPP
    switch (tex->bpp)
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
    glCreateTextures(tex->target, 1, &tex->texture_id);
    glBindTexture(tex->target, tex->texture_id);

    // Fonts should be rendered at native resolution, so no need for filtering
    glTexParameteri(tex->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(tex->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Stop chararcters from bleeding over edges
    glTexParameteri(tex->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(tex->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Tex creation params are dependent on BPP
    switch (font->RenderStyle)
    {
    case BFG_RS_ALPHA:
        glTexImage2D(tex->target, 0, GL_RED, tex->width, tex->height, 0,
                     GL_RED, GL_UNSIGNED_BYTE, &buffer[MAP_DATA_OFFSET]);
        break;

    case BFG_RS_RGB:
        glTexImage2D(tex->target, 0, GL_RGB, tex->width, tex->height, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, &buffer[MAP_DATA_OFFSET]);
        break;

    case BFG_RS_RGBA:
        glTexImage2D(tex->target, 0, GL_RGBA, tex->width, tex->height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, &buffer[MAP_DATA_OFFSET]);
        break;
    }

    // TODO: Do we want font mipmaps? Probably not...
    //glGenerateMipmap(tex->target);

    font->tex = tex;

cleanup:
    free(buffer);
    glBindTexture(tex->target, 0);
    return font;
}

void font_setblend(const struct rico_font *font)
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

void font_render(const struct rico_font *font, int x, int y,
                 const char *text, struct col4 bg,
                 struct rico_mesh **out_mesh,
                 struct rico_texture **out_texture)
{
    // UNUSED(font);
    // UNUSED(x);
    // UNUSED(y);
    // UNUSED(text);
    // UNUSED(bg);

    //font_setblend(font);

    int text_len = strlen(text);
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
    int cur_y = y;

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

        // Cleanup: DIRTY233d
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
            (struct vec4) { cur_x / 64.0f,
                            cur_y / 64.0f, 0.0f, 1.0f },
            bg,
            (struct tex2) { u0, v1 }
        };
        vertices[idx_vertex++] = (struct mesh_vertex) {
            (struct vec4) { (cur_x + xOffset) / 64.0f,
                             cur_y / 64.0f, 0.0f, 1.0f },
            bg,
            (struct tex2) { u1, v1 }
        };
        vertices[idx_vertex++] = (struct mesh_vertex) {
            (struct vec4) { (cur_x + xOffset) / 64.0f,
                            (cur_y + font->YOffset) / 64.0f, 0.0f, 1.0f },
            bg,
            (struct tex2) { u1, v0 }
        };
        vertices[idx_vertex++] = (struct mesh_vertex) {
            (struct vec4) { cur_x / 64.0f,
                           (cur_y + font->YOffset) / 64.0f, 0.0f, 1.0f },
            bg,
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

    for (int i = 0; i < 8; i++)
    {
        printf("v %f %f %f %f %f\n", vertices[i].pos.x, vertices[i].pos.y, vertices[i].pos.z, vertices[i].uv.u, vertices[i].uv.v);
    }

    for (int i = 0; i < 12; i++)
    {
        printf("e %d\n", elements[i]);
    }

    (*out_mesh) = make_mesh("font", make_program_default(), vertices, vertex_count,
                         elements, element_count, GL_STATIC_DRAW);
    (*out_texture) = font->tex;

    free(vertices);
    free(elements);

    ///////////////////////////////////////////////////
    // Test font render immediately
    //struct mat4 model_matrix;
    //mat4_ident(&model_matrix);
    //mat4_translate(&model_matrix, VEC4_ZERO);
    //mat4_rotx(&model_matrix, 0.0f);
    //mat4_roty(&model_matrix, 0.0f);
    //mat4_rotz(&model_matrix, 0.0f);
    //mat4_scale(&model_matrix, VEC4_UNIT);
//
    //mesh_render(mesh, &model_matrix, VEC4_UNIT);
    ///////////////////////////////////////////////////
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