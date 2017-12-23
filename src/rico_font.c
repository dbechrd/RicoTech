struct bff_header
{
    unsigned char ID1, ID2;
    unsigned char BPP;
    int ImageWidth, ImageHeight, CellWidth, CellHeight;
    unsigned char StartPoint;
};

global const char *font_name(struct rico_font *font)
{
    return (u8 *)font + font->name_offset;
}
internal struct rico_texture *font_texture(struct rico_font *font)
{
    return (struct rico_texture *)((u8 *)font + font->texture_offset);
}

#if 0
int font_free(struct rico_font *font)
{
    enum rico_error err;

#if RICO_DEBUG_FONT
    printf("[font][free] uid=%d name=%s\n", font->hnd.uid, font->hnd.name);
#endif

    err = chunk_free(font->hnd.chunk, font->texture_offset);
    if (err) return err;
    err = pool_remove(font->hnd.pool, font->hnd.id);
    return err;
}
#endif

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

void font_render(struct rico_mesh **mesh, struct rico_texture **texture,
                 struct rico_font *font, int x, int y, struct col4 bg,
                 const char *text, const char *mesh_name,
                 enum rico_mesh_type type)
{
    // TODO: Push/pop these on arena instead of using local stack? Does it
    //       matter?
    // Persistent buffers for font rendering
    local struct rico_vertex vertices[BFG_MAXSTRING * 4] = { 0 };
    local GLuint elements[BFG_MAXSTRING * 6] = { 0 };

    if (!font)
    {
        font = pack_read(pack_default, RICO_DEFAULT_FONT);
    }

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
        vertices[idx_vertex++] = (struct rico_vertex) {
            (struct vec3) {
                cur_x / 64.0f,
                cur_y / 64.0f,
                0.0f
            },
            bg,
            (struct vec3) { 1.0f, 1.0f, 1.0f },
            (struct tex2) { u0, v1 }
        };
        vertices[idx_vertex++] = (struct rico_vertex) {
            (struct vec3) {
                (cur_x + xOffset) / 64.0f,
                cur_y / 64.0f,
                0.0f
            },
            bg,
            (struct vec3) { 1.0f, 1.0f, 1.0f },
            (struct tex2) { u1, v1 }
        };
        vertices[idx_vertex++] = (struct rico_vertex) {
            (struct vec3) {
                (cur_x + xOffset) / 64.0f,
                (cur_y + font->YOffset) / 64.0f,
                0.0f
            },
            bg,
            (struct vec3) { 1.0f, 1.0f, 1.0f },
            (struct tex2) { u1, v0 }
        };
        vertices[idx_vertex++] = (struct rico_vertex) {
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

    // TODO: This stuff is severely broken, need to figure out where these
    //       dynamic-at-runtime meshes will go for e.g. screen strings.
    u32 mesh_id = load_mesh(pack_frame, mesh_name, idx_vertex, vertices,
                            idx_element, elements);
    *mesh = pack_read(pack_frame, mesh_id);
    *texture = font_texture(font);
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
