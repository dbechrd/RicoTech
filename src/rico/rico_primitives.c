//struct pool_id PRIM_MESH_BBOX;
//struct pool_id PRIM_MESH_SPHERE;

static GLuint prim_line_vao;
static GLuint prim_line_vbo;

static GLuint prim_bbox_vao;
static GLuint prim_bbox_vbo[VBO_COUNT];

static int prim_init()
{
    enum RICO_error err;

    ///-------------------------------------------------------------------------
    //| Line / Ray
    ///-------------------------------------------------------------------------
    err = make_program_primitive(&prog_prim);
    if (err) return err;

    // Generate VAO and buffers
    glGenVertexArrays(1, &prim_line_vao);
    glBindVertexArray(prim_line_vao);

    glGenBuffers(1, &prim_line_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, prim_line_vbo);

    // Shader attribute pointers
#if 1
    glVertexAttribPointer(LOCATION_PRIM_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(LOCATION_PRIM_POSITION);
#else
    glVertexAttribPointer(LOCATION_PRIM_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, pos));
    glEnableVertexAttribArray(LOCATION_PRIM_POSITION);

    glVertexAttribPointer(LOCATION_PRIM_COLOR, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, col));
    glEnableVertexAttribArray(LOCATION_PRIM_COLOR);
#endif

    ///-------------------------------------------------------------------------
    //| Bounding box
    ///-------------------------------------------------------------------------
    glGenVertexArrays(1, &prim_bbox_vao);
    glBindVertexArray(prim_bbox_vao);

    glGenBuffers(2, prim_bbox_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, prim_bbox_vbo[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim_bbox_vbo[1]);

    const GLushort elements[] = {
        0, 1, 2, 3,            // front face
        4, 5, 6, 7,            // back face
        0, 4, 1, 5, 2, 6, 3, 7 // 4 edges
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(
        LOCATION_PRIM_POSITION, // attribute
        3,                      // number of elements per vertex
        GL_FLOAT,               // the type of each element
        GL_FALSE,               // take our values as-is
        0,                      // no extra data between each position
        0                       // offset of first element
    );
    glEnableVertexAttribArray(LOCATION_PRIM_POSITION);

    ///-------------------------------------------------------------------------
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return err;
}
// TODO: Queue up primitive requests and batch them within a single
//       glUseProgram() call.
extern void RICO_prim_draw_line(const struct vec3 *p0, const struct vec3 *p1,
                                const struct mat4 *matrix,
                                const struct vec4 *color)
{
    struct vec3 vertices[2] = { *p0, *p1 };

    RICO_ASSERT(prog_prim->prog_id);
    glUseProgram(prog_prim->prog_id);
    glUniformMatrix4fv(prog_prim->u_proj, 1, GL_TRUE, cam_player.proj_matrix.a);
    glUniformMatrix4fv(prog_prim->u_view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(prog_prim->u_model, 1, GL_TRUE, matrix->a);
    glUniform4f(prog_prim->u_col, color->r, color->g, color->b, color->a);

    RICO_ASSERT(prim_line_vao);
    RICO_ASSERT(prim_line_vbo);
    glBindVertexArray(prim_line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, prim_line_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINES, 0, 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}
// Render ray as line segment
extern void RICO_prim_draw_ray(const struct ray *ray, const struct mat4 *matrix,
                               const struct vec4 *color)
{
    struct vec3 ray_end = ray->orig;
    v3_add(&ray_end, &ray->dir);
    RICO_prim_draw_line(&ray->orig, &ray->dir, matrix, color);
}
extern void RICO_prim_draw_quad(const struct quad *quad,
                                const struct mat4 *matrix,
                                const struct vec4 *color)
{
    RICO_ASSERT(prog_prim->prog_id);
    glUseProgram(prog_prim->prog_id);
    glUniformMatrix4fv(prog_prim->u_proj, 1, GL_TRUE, cam_player.proj_matrix.a);
    glUniformMatrix4fv(prog_prim->u_view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(prog_prim->u_model, 1, GL_TRUE, matrix->a);
    glUniform4fv(prog_prim->u_col, 1, (const GLfloat *)color);

    RICO_ASSERT(prim_line_vao);
    RICO_ASSERT(prim_line_vbo);
    glBindVertexArray(prim_line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, prim_line_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad->verts), quad->verts,
                 GL_STATIC_DRAW);

    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glEnable(GL_CULL_FACE);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Clean up
    glUseProgram(0);
}
extern void RICO_prim_draw_plane(const struct vec3 *n,
                                 const struct mat4 *matrix,
                                 const struct vec4 *color)
{
    struct vec3 tx = VEC3(n->y - n->z, n->z - n->x, n->x - n->y);
    struct vec3 ty = v3_cross(&tx, n);

    struct quad quad = { 0 };
    quad.verts[0] = ty;
    quad.verts[1] = tx;
    quad.verts[2] = *v3_negate(&tx);
    quad.verts[3] = *v3_negate(&ty);

    for (int i = 0; i < 4; ++i)
    {
        v3_scalef(&quad.verts[i], 10.0f);
    }

    RICO_prim_draw_quad(&quad, matrix, color);
}
extern void RICO_prim_draw_bbox(const struct RICO_bbox *bbox,
                                const struct mat4 *matrix,
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

    RICO_ASSERT(prog_prim->prog_id);
    glUseProgram(prog_prim->prog_id);

    glUniformMatrix4fv(prog_prim->u_proj, 1, GL_TRUE, cam_player.proj_matrix.a);
    glUniformMatrix4fv(prog_prim->u_view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(prog_prim->u_model, 1, GL_TRUE, matrix->a);

    glUniform4fv(prog_prim->u_col, 1, (const GLfloat *)color);

    RICO_ASSERT(prim_bbox_vao);
    RICO_ASSERT(prim_bbox_vbo[0]);
    RICO_ASSERT(prim_bbox_vbo[1]);
    glBindVertexArray(prim_bbox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, prim_bbox_vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim_bbox_vbo[1]);

    // Based loosely on:
    // https://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT,
                   (GLvoid*)(4 * sizeof(GLushort)));
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT,
                   (GLvoid*)(8 * sizeof(GLushort)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Clean up
    glUseProgram(0);
}
extern void RICO_prim_draw_obb(const struct RICO_obb *obb,
                               const struct vec4 *color)
{
    struct vec3 e0 = obb->u[0];
    v3_scalef(&e0, obb->e.a[0]);
    struct vec3 e1 = obb->u[1];
    v3_scalef(&e1, obb->e.a[1]);
    struct vec3 e2 = obb->u[2];
    v3_scalef(&e2, obb->e.a[2]);

    // DEBUG: Draw obb axes
    struct vec3 axis0 = obb->c;
    struct vec3 axis1 = obb->c;
    struct vec3 axis2 = obb->c;
    v3_add(&axis0, &e0);
    v3_add(&axis1, &e1);
    v3_add(&axis2, &e2);
    RICO_prim_draw_line(&obb->c, &axis0, &MAT4_IDENT, &COLOR_RED);
    RICO_prim_draw_line(&obb->c, &axis1, &MAT4_IDENT, &COLOR_GREEN);
    RICO_prim_draw_line(&obb->c, &axis2, &MAT4_IDENT, &COLOR_BLUE);

    struct vec3 p0 = obb->c; // 000
    v3_sub(&p0, &e0);
    v3_sub(&p0, &e1);
    v3_sub(&p0, &e2);
    struct vec3 p1 = obb->c; // 100
    v3_add(&p1, &e0);
    v3_sub(&p1, &e1);
    v3_sub(&p1, &e2);
    struct vec3 p2 = obb->c; // 110
    v3_add(&p2, &e0);
    v3_add(&p2, &e1);
    v3_sub(&p2, &e2);
    struct vec3 p3 = obb->c; // 010
    v3_sub(&p3, &e0);
    v3_add(&p3, &e1);
    v3_sub(&p3, &e2);
    struct vec3 p4 = obb->c; // 001
    v3_sub(&p4, &e0);
    v3_sub(&p4, &e1);
    v3_add(&p4, &e2);
    struct vec3 p5 = obb->c; // 101
    v3_add(&p5, &e0);
    v3_sub(&p5, &e1);
    v3_add(&p5, &e2);
    struct vec3 p6 = obb->c; // 111
    v3_add(&p6, &e0);
    v3_add(&p6, &e1);
    v3_add(&p6, &e2);
    struct vec3 p7 = obb->c; // 011
    v3_sub(&p7, &e0);
    v3_add(&p7, &e1);
    v3_add(&p7, &e2);

    GLfloat vertices[] = {
        p0.x, p0.y, p0.z,
        p1.x, p1.y, p1.z,
        p2.x, p2.y, p2.z,
        p3.x, p3.y, p3.z,
        p4.x, p4.y, p4.z,
        p5.x, p5.y, p5.z,
        p6.x, p6.y, p6.z,
        p7.x, p7.y, p7.z
    };

    RICO_ASSERT(prog_prim->prog_id);
    glUseProgram(prog_prim->prog_id);

    glUniformMatrix4fv(prog_prim->u_proj, 1, GL_TRUE, cam_player.proj_matrix.a);
    glUniformMatrix4fv(prog_prim->u_view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(prog_prim->u_model, 1, GL_TRUE, MAT4_IDENT.a);

    glUniform4fv(prog_prim->u_col, 1, (const GLfloat *)color);

    RICO_ASSERT(prim_bbox_vao);
    RICO_ASSERT(prim_bbox_vbo[0]);
    RICO_ASSERT(prim_bbox_vbo[1]);
    glBindVertexArray(prim_bbox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, prim_bbox_vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim_bbox_vbo[1]);

    // Based loosely on:
    // https://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT,
        (GLvoid*)(4 * sizeof(GLushort)));
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT,
        (GLvoid*)(8 * sizeof(GLushort)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Clean up
    glUseProgram(0);
}
extern void RICO_prim_draw_sphere(const struct sphere *sphere,
                                  const struct vec4 *color)
{
    if (cam_player.fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    struct mat4 matrix = MAT4_IDENT;
    mat4_translate(&matrix, &sphere->orig);
    mat4_scalef(&matrix, sphere->radius);

    RICO_ASSERT(prog_prim->prog_id);
    glUseProgram(prog_prim->prog_id);

    glUniformMatrix4fv(prog_prim->u_proj, 1, GL_TRUE, cam_player.proj_matrix.a);
    glUniformMatrix4fv(prog_prim->u_view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(prog_prim->u_model, 1, GL_TRUE, matrix.a);
    glUniform4f(prog_prim->u_col, color->r, color->g, color->b, color->a);

    mesh_render(MESH_DEFAULT_SPHERE, PROG_PRIM);

    // Clean up
    glUseProgram(0);

    if (cam_player.fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, cam_player.fill_mode);
}
static void prim_free()
{
    // TODO: Clean-up prim VAO / VBO? Will probably just keep them for life
    //       of the application for now.
}
