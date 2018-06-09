struct bff_header
{
    unsigned char ID1, ID2;
    unsigned char BPP;
    int ImageWidth, ImageHeight, CellWidth, CellHeight;
    unsigned char StartPoint;
};

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

static void font_setblend(const struct RICO_font *font)
{
    // TODO: Preserve blend settings before changing
	switch(font->render_style)
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
static void font_render(u32 *mesh_id, u32 *tex_id, pkid font_id, float x,
                        float y, struct vec4 bg, const char *text,
                        const char *mesh_name)
{
    // TODO: Use instanced quad?
    // Persistent buffers for font rendering
    static struct text_vertex vertices[BFG_MAXSTRING * 4] = { 0 };
    static GLuint elements[BFG_MAXSTRING * 6] = { 0 };

    struct RICO_font *font;
    if (font_id)
    {
        font = RICO_pack_lookup(font_id);
    }
    else
    {
        font = RICO_pack_lookup(FONT_DEFAULT);
    }

    //font_setblend(font);

    int text_len = dlb_strlen(text);

    // Truncate strings that are too long to render
    if (text_len > BFG_MAXSTRING)
        text_len = BFG_MAXSTRING;

    int idx_vertex = 0;
    int idx_element = 0;

    float screen_x = x;
    float screen_y = y;

    for (int i = 0; i < text_len; i++)
    {
        if (text[i] == '\n') {
            screen_y += SCREEN_H(font->y_offset);
            screen_x = x;
            continue;
        }

        int row = (text[i] - font->base_char) / font->row_pitch;
        int col = (text[i] - font->base_char) - (row * font->row_pitch);

        GLfloat u0 = col * font->col_factor;
        GLfloat v0 = row * font->row_factor;
        GLfloat u1 = u0 + font->col_factor;
        GLfloat v1 = v0 + font->row_factor;

        if (font->y_invert)
        {
            GLfloat tmp = v0;
            v0 = v1;
            v1 = tmp;
        }

        int char_width = font->char_widths[(int)text[i]];
        float offset_x = SCREEN_W(char_width);
        float offset_y = SCREEN_H(font->cell_y);

        // Vertices for this character's quad
        vertices[idx_vertex++] = (struct text_vertex) {
            VEC2F(screen_x, screen_y + offset_y),
            VEC2F(u0, v1), bg
        };
        vertices[idx_vertex++] = (struct text_vertex) {
            VEC2F(screen_x + offset_x, screen_y + offset_y),
            VEC2F(u1, v1), bg
        };
        vertices[idx_vertex++] = (struct text_vertex) {
            VEC2F(screen_x + offset_x, screen_y),
            VEC2F(u1, v0), bg
        };
        vertices[idx_vertex++] = (struct text_vertex) {
            VEC2F(screen_x, screen_y),
            VEC2F(u0, v0), bg
        };

        // Triangles using this character's vertices
        elements[idx_element++] = idx_vertex - 4;
        elements[idx_element++] = idx_vertex - 3;
        elements[idx_element++] = idx_vertex - 2;

        elements[idx_element++] = idx_vertex - 4;
        elements[idx_element++] = idx_vertex - 2;
        elements[idx_element++] = idx_vertex - 1;

        screen_x += offset_x;
    }

    pkid new_mesh_id =
        RICO_load_mesh(PACK_TRANSIENT, mesh_name, sizeof(*vertices), idx_vertex,
                       vertices, idx_element, elements, PROG_TEXT);

    RICO_ASSERT(new_mesh_id);
    *mesh_id = new_mesh_id;
    *tex_id = font->tex_id;
}

/*

bool Load(char *fname);
static void SetScreen(int x, int y);
static void SetCursor(int x, int y);
static void SetColor(float Red, float Green, float Blue);
static void ReverseYAxis(bool State);
static void Select();
static void Bind();
static void SetBlend();
static void Print(char *Text);
static void Print(char *Text, int x, int y);
static void ezPrint(char *Text, int x, int y);
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
