const char *rico_prim_string[] = {
    RICO_PRIMITIVES(GEN_STRING)
};

struct pool_id PRIM_MESH_BBOX;
struct pool_id PRIM_MESH_SPHERE;

global GLuint vaos[PRIM_COUNT];
global GLuint vbos[PRIM_COUNT][VBO_COUNT];

global struct program_primitive *program;

internal int prim_init_gl(enum rico_prim prim);

int prim_init(enum rico_prim prim)
{
    enum rico_error err = make_program_primitive(&program);
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
    glUseProgram(program->prog_id);

    // Transform
    glUniformMatrix4fv(program->u_proj, 1, GL_TRUE, cam_player.proj_matrix.a);
    glUniformMatrix4fv(program->u_view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(program->u_model, 1, GL_TRUE, model_matrix->a);

    glUniform4f(program->u_col, color.r, color.g, color.b, color.a);

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

void prim_draw_bbox_color(const struct bbox *bbox,
                          const struct rico_transform *model_xform,
                          const struct vec4 *color)
{
    if (bbox->wireframe && cam_player.fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    struct mat4 transform = MAT4_IDENT;

    // Adjust unit bbox to fit mesh and model transform
    //--------------------------------------------------
    struct vec3 center = bbox->min;
    v3_add(&center, &bbox->max);
    v3_scalef(&center, 0.5f);
    
    mat4_translate(&transform, &center);
    mat4_translate(&transform, &BBOX_EPSILON_TRANS);

    mat4_translate(&transform, &model_xform->trans);
    mat4_rotx(&transform, model_xform->rot.x);
    mat4_roty(&transform, model_xform->rot.y);
    mat4_rotz(&transform, model_xform->rot.z);
    mat4_scale(&transform, &model_xform->scale);
    
    struct vec3 scale = bbox->max;
    v3_sub(&scale, &bbox->min);
    mat4_scale(&transform, &scale);
    //--------------------------------------------------

    glUseProgram(program->prog_id);

    glUniformMatrix4fv(program->u_proj, 1, GL_TRUE, cam_player.proj_matrix.a);
    glUniformMatrix4fv(program->u_view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(program->u_model, 1, GL_TRUE, transform.a);

    // TODO: Use per-bbox color instead of rainbowgasm
    glUniform4f(program->u_col, color->r, color->g, color->b, color->a);
	//UNUSED(color);
    //glUniform4f(program->u_col, 1.0f, 1.0f, 1.0f, 0.5f);

    mesh_render(pack_default, MESH_DEFAULT_CUBE, OBJ_NULL);

    // Clean up
    glUseProgram(0);

    if (bbox->wireframe && cam_player.fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, cam_player.fill_mode);
}

void prim_draw_sphere(const struct sphere *sphere, const struct vec4 *color)
{
    if (cam_player.fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    struct mat4 model_matrix = MAT4_IDENT;
    mat4_translate(&model_matrix, &sphere->orig);
    mat4_scalef(&model_matrix, sphere->radius);

    glUseProgram(program->prog_id);

    glUniformMatrix4fv(program->u_proj, 1, GL_TRUE, cam_player.proj_matrix.a);
    glUniformMatrix4fv(program->u_view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(program->u_model, 1, GL_TRUE, model_matrix.a);
    glUniform4f(program->u_col, color->r, color->g, color->b, color->a);

    mesh_render(pack_default, MESH_DEFAULT_SPHERE, OBJ_NULL);

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
