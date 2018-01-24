const char *rico_prim_string[] = {
    RICO_PRIMITIVES(GEN_STRING)
};

struct pool_id PRIM_MESH_BBOX;
struct pool_id PRIM_MESH_SPHERE;

global GLuint vaos[PRIM_COUNT];
global GLuint vbos[PRIM_COUNT][VBO_COUNT];

internal int prim_init_gl(enum rico_prim prim);

int prim_init(enum rico_prim prim)
{
    enum rico_error err = make_program_primitive(&prog_prim);
    if (err) return err;

    return prim_init_gl(prim);
}

internal int prim_init_gl(enum rico_prim prim)
{
    //--------------------------------------------------------------------------
    // Generate VAO and buffers
    //--------------------------------------------------------------------------
    glGenVertexArrays(1, &vaos[prim]);
    glBindVertexArray(vaos[prim]);

    int buffers;
    switch (prim)
    {
    case PRIM_SEGMENT:
    case PRIM_RAY:
        buffers = 1;
        break;
    default:
        return ERR_PRIM_UNSUPPORTED;
    }
    glGenBuffers(buffers, vbos[prim]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[prim][VBO_VERTEX]);

    //--------------------------------------------------------------------------
    // Shader attribute pointers
    //--------------------------------------------------------------------------
    glVertexAttribPointer(LOCATION_PBR_POSITION, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, pos));
    glEnableVertexAttribArray(LOCATION_PBR_POSITION);

    glVertexAttribPointer(LOCATION_PBR_COLOR, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, col));
    glEnableVertexAttribArray(LOCATION_PBR_COLOR);

    // Clean up
    glBindVertexArray(0);
    return SUCCESS;
}

// TODO: Queue up primitive requests and batch them within a single
//       glUseProgram() call.
// Render line segment
void prim_draw_segment(const struct segment *seg,
                       const struct mat4 *model_matrix, struct vec4 color)
{
    if (cam_player.fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Set shader program
    glUseProgram(prog_prim->prog_id);

    // Transform
    glUniformMatrix4fv(prog_prim->u_proj, 1, GL_TRUE, cam_player.proj_matrix.a);
    glUniformMatrix4fv(prog_prim->u_view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(prog_prim->u_model, 1, GL_TRUE, model_matrix->a);

    glUniform4f(prog_prim->u_col, color.r, color.g, color.b, color.a);

    // Draw
    glBindVertexArray(vaos[PRIM_SEGMENT]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[PRIM_SEGMENT][VBO_VERTEX]);
    // TODO: Batch these into a single glDrawArrays(LINES, 0, 2 * segment_count)
    glBufferData(GL_ARRAY_BUFFER, sizeof(seg->vertices), seg->vertices,
                 GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINES, 0, 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    if (cam_player.fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, cam_player.fill_mode);
}

// Render ray as line segment
void prim_draw_ray(const struct ray *ray, const struct mat4 *model_matrix,
                   struct vec4 color)
{
    struct vec3 ray_end = ray->orig;
    v3_add(&ray_end, &ray->dir);

    struct segment ray_seg;
    ray_seg.vertices[0] = (struct prim_vertex) { ray->orig, COLOR_GRAY };
    ray_seg.vertices[1] = (struct prim_vertex) { ray_end, COLOR_WHITE };

    prim_draw_segment(&ray_seg, model_matrix, color);
}

void prim_draw_bbox(const struct bbox *bbox,
                    const struct rico_transform *model_xform)
{
    prim_draw_bbox_color(bbox, model_xform, &bbox->color);
}

static GLuint bbox_vao = 0;
static GLuint bbox_buffers[2] = { 0 };

void prim_bbox_init()
{
    // TODO: Delete vao / buffers?
    glGenVertexArrays(1, &bbox_vao);
    glBindVertexArray(bbox_vao);
    
    glGenBuffers(2, bbox_buffers);

    glBindBuffer(GL_ARRAY_BUFFER, bbox_buffers[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bbox_buffers[1]);

    const GLushort elements[] = {
        0, 1, 2, 3, // front face
        4, 5, 6, 7, // back face
        0, 4, 1, 5, 2, 6, 3, 7 // 4 edges
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    glVertexAttribPointer(
        LOCATION_PRIM_POSITION,    // attribute
        3,                  // number of elements per vertex
        GL_FLOAT,           // the type of each element
        GL_FALSE,           // take our values as-is
        0,                  // no extra data between each position
        0                   // offset of first element
    );
    glEnableVertexAttribArray(LOCATION_PRIM_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void prim_draw_bbox_color(const struct bbox *bbox,
                          const struct rico_transform *model_xform,
                          const struct vec4 *color)
{
    GLfloat vertices[] = {
        bbox->min.x, bbox->min.y, bbox->min.z,
        bbox->max.x, bbox->min.y, bbox->min.z,
        bbox->max.x, bbox->max.y, bbox->min.z,
        bbox->min.x, bbox->max.y, bbox->min.z,
        bbox->min.x, bbox->min.y, bbox->max.z,
        bbox->max.x, bbox->min.y, bbox->max.z,
        bbox->max.x, bbox->max.y, bbox->max.z,
        bbox->min.x, bbox->max.y, bbox->max.z,
    };

    struct mat4 transform = model_xform->matrix; //MAT4_IDENT;

    glUseProgram(prog_prim->prog_id);

    glUniformMatrix4fv(prog_prim->u_proj, 1, GL_TRUE, cam_player.proj_matrix.a);
    glUniformMatrix4fv(prog_prim->u_view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(prog_prim->u_model, 1, GL_TRUE, transform.a);

    if (bbox->selected)
        glUniform4fv(prog_prim->u_col, 1, (const GLfloat *)&COLOR_RED);
    else
        glUniform4fv(prog_prim->u_col, 1, (const GLfloat *)color);

    glBindVertexArray(bbox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, bbox_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bbox_buffers[1]);

    // Based loosely on:
    // https://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4 * sizeof(GLushort)));
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(GLushort)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Clean up
    glUseProgram(0);
}

void prim_draw_sphere(const struct sphere *sphere, const struct vec4 *color)
{
    if (cam_player.fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    struct mat4 model_matrix = MAT4_IDENT;
    mat4_translate(&model_matrix, &sphere->orig);
    mat4_scalef(&model_matrix, sphere->radius);

    glUseProgram(prog_prim->prog_id);

    glUniformMatrix4fv(prog_prim->u_proj, 1, GL_TRUE, cam_player.proj_matrix.a);
    glUniformMatrix4fv(prog_prim->u_view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(prog_prim->u_model, 1, GL_TRUE, model_matrix.a);
    glUniform4f(prog_prim->u_col, color->r, color->g, color->b, color->a);

    mesh_render(packs[PACK_DEFAULT], MESH_DEFAULT_SPHERE, PROG_PRIM);

    // Clean up
    glUseProgram(0);

    if (cam_player.fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, cam_player.fill_mode);
}

void prim_free(enum rico_prim prim)
{
    // TODO: Clean-up prim VAO / VBO? Will probably just keep them for life
    //       of the application for now.
    UNUSED(prim);
}
