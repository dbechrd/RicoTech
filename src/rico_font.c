const u32 RICO_FONT_SIZE = sizeof(struct rico_font);

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

struct hnd RICO_DEFAULT_FONT = { 0 };

internal inline struct rico_pool **font_pool_ptr(enum rico_persist persist)
{
    struct rico_chunk *chunk = chunk_active();
    RICO_ASSERT(chunk);
    RICO_ASSERT(chunk->pools[persist][POOL_FONTS]);
    return &chunk->pools[persist][POOL_FONTS];
}

internal inline struct rico_pool *font_pool(enum rico_persist persist)
{
    return *font_pool_ptr(persist);
}

internal inline struct rico_font *font_find(struct hnd handle)
{
    struct rico_font *font = pool_read(font_pool(handle.persist), handle.value);
    RICO_ASSERT(font->uid.uid);
    return font;
}

int font_request_by_name(struct hnd *_handle, enum rico_persist persist,
                         const char *name)
{
    struct hnd handle = hashtable_search_by_name(&global_fonts, name);
    if (!handle.value)
    {
        return RICO_ERROR(ERR_FONT_INVALID_NAME, "Font not found: %s.", name);
    }

    *_handle = handle;
    return SUCCESS;
}

int font_init(struct hnd *_handle, enum rico_persist persist,
              const char *filename)
{
    enum rico_error err;

#if RICO_DEBUG_FONT
    printf("[font][init] filename=%s\n", filename);
#endif

    struct hnd handle;
    struct rico_font *font;
    err = pool_handle_alloc(font_pool_ptr(persist), &handle, &font);
    if (err) return err;

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

    struct hnd tex_handle;
    err = texture_load_pixels(&tex_handle, persist, filename, GL_TEXTURE_2D,
                              width, height, bpp, &buffer[MAP_DATA_OFFSET]);
    if (err) goto cleanup;
    font->texture = texture_request(tex_handle);

    if (_handle) *_handle = handle;

cleanup:
    free(buffer);
    return err;
}

void font_free(struct hnd handle)
{
    struct rico_font *font = font_find(handle);

    // Cleanup: Use fixed pool slots
    //if (handle == RICO_DEFAULT_FONT)
    //    return;

#if RICO_DEBUG_FONT
    printf("[font][free] uid=%d name=%s\n", font->uid.uid, font->uid.name);
#endif

    texture_free(font->texture);

    font->uid.uid = UID_NULL;
    pool_handle_free(font_pool(handle.persist), handle);
}

internal void font_setblend(const struct rico_font *font)
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

int font_render(struct hnd *_mesh, struct hnd *_texture,
                struct hnd handle, int x, int y, struct col4 bg,
                const char *text, const char *mesh_name,
                enum rico_mesh_type type)
{
    // Persistent buffers for font rendering
    local struct mesh_vertex vertices[BFG_MAXSTRING * 4];
    local GLuint elements[BFG_MAXSTRING * 6];

    // Cleanup: Debug code
    memset(vertices, 0, sizeof(vertices));
    memset(elements, 0, sizeof(elements));

    enum rico_error err;

    if (!handle.value) handle = RICO_DEFAULT_FONT;

    struct rico_font *font = font_find(handle);
    RICO_ASSERT(font->uid.uid != UID_NULL);

    //font_setblend(font);

    int text_len = strlen(text);

    // Truncate strings that are too long to render
    if (text_len > BFG_MAXSTRING)
        text_len = BFG_MAXSTRING;

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

    struct hnd mesh_handle;
    err = mesh_load(&mesh_handle, handle.persist, mesh_name, type, idx_vertex,
                    vertices, idx_element, elements, GL_STATIC_DRAW);
    if (err) goto cleanup;
    
    *_mesh = mesh_handle;
    *_texture = font->texture;

cleanup:
    //free(vertices);
    //free(elements);
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