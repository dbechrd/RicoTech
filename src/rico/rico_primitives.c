//struct pool_id PRIM_MESH_BBOX;
//struct pool_id PRIM_MESH_SPHERE;

static GLuint prim_line_vao;
static GLuint prim_line_vbo;

static GLuint prim_quad_vao;
static GLuint prim_quad_vbo;

static GLuint prim_bbox_vao;
static GLuint prim_bbox_vbo[VBO_COUNT];

static int prim_init()
{
    enum RICO_error err;

    err = make_program_primitive(&prog_prim);
    if (err) return err;

    ///-------------------------------------------------------------------------
    //| Line / Ray
    ///-------------------------------------------------------------------------
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

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ///-------------------------------------------------------------------------
    //| Quad
    ///-------------------------------------------------------------------------
    // Generate VAO and buffers
    glGenVertexArrays(1, &prim_quad_vao);
    glBindVertexArray(prim_quad_vao);

    glGenBuffers(1, &prim_quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, prim_quad_vbo);

    // Shader attribute pointers
    glVertexAttribPointer(LOCATION_PRIM_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, pos));
    glEnableVertexAttribArray(LOCATION_PRIM_POSITION);

    glVertexAttribPointer(LOCATION_PRIM_UV, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, uv));
    glEnableVertexAttribArray(LOCATION_PRIM_UV);

    glVertexAttribPointer(LOCATION_PRIM_COLOR, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, col));
    glEnableVertexAttribArray(LOCATION_PRIM_COLOR);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return err;
}
// TODO: Queue up primitive requests and batch them within a single
//       glUseProgram() call.
extern void RICO_prim_draw_line2d(float x0, float y0, float x1, float y1,
                                  const struct vec4 *color)
{
    const float ortho_z = -1.0f;
    prim_draw_line(&VEC3(x0, y0, ortho_z), &VEC3(x1, y1, ortho_z),
                   color, &MAT4_IDENT, &MAT4_IDENT, &cam_player.ortho_matrix);
}
extern void RICO_prim_draw_line(const struct vec3 *p0, const struct vec3 *p1,
                                const struct vec4 *color)
{
    prim_draw_line(p0, p1, color, &MAT4_IDENT, &cam_player.view_matrix,
                   cam_player.proj_matrix);
}
extern void RICO_prim_draw_line_xform(const struct vec3 *p0,
                                      const struct vec3 *p1,
                                      const struct vec4 *color,
                                      const struct mat4 *xform)
{
    prim_draw_line(p0, p1, color, xform, &cam_player.view_matrix,
                   cam_player.proj_matrix);
}
static void prim_draw_line(const struct vec3 *p0, const struct vec3 *p1,
                           const struct vec4 *color, const struct mat4 *xform,
                           const struct mat4 *view, const struct mat4 *proj)
{
    struct vec3 vertices[2] = { *p0, *p1 };

    RICO_ASSERT(prog_prim->program.gl_id);
    glUseProgram(prog_prim->program.gl_id);
    glUniformMatrix4fv(prog_prim->vert.proj, 1, GL_TRUE, proj->a);
    glUniformMatrix4fv(prog_prim->vert.view, 1, GL_TRUE, view->a);
    glUniformMatrix4fv(prog_prim->vert.model, 1, GL_TRUE, xform->a);

    //glUniform4fv(prog_prim->col, 1, (const GLfloat *)color);
    // TODO: Bind texture

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
extern void RICO_prim_draw_ray(const struct ray *ray, const struct vec4 *color)
{
    RICO_prim_draw_ray_xform(ray, color, &MAT4_IDENT);
}
extern void RICO_prim_draw_ray_xform(const struct ray *ray,
                                     const struct vec4 *color,
                                     const struct mat4 *xform)
{
    struct vec3 ray_end = ray->orig;
    v3_add(&ray_end, &ray->dir);
    RICO_prim_draw_line_xform(&ray->orig, &ray->dir, color, xform);
}
extern void RICO_prim_draw_rect(const struct rect *rect,
                                const struct vec4 *color)
{
    RICO_prim_draw_rect_tex(rect, color, 0);
}
extern void RICO_prim_draw_rect_tex(const struct rect *rect,
                                    const struct vec4 *color, pkid tex_pkid)
{
    // (0,0) (0,1) (1,0) (1,1)
    float x0 = SCREEN_X(rect->x);
    float y0 = SCREEN_Y(rect->y);
    float x1 = SCREEN_X(rect->x + rect->w);
    float y1 = SCREEN_Y(rect->y + rect->h);
    const float ortho_z = -1.0f;

    struct quad quad;
    quad.verts[0] = VEC3(x0, y1, ortho_z);
    quad.verts[1] = VEC3(x0, y0, ortho_z);
    quad.verts[2] = VEC3(x1, y1, ortho_z);
    quad.verts[3] = VEC3(x1, y0, ortho_z);
    prim_draw_quad(&quad, color, &MAT4_IDENT, &MAT4_IDENT,
                   &cam_player.ortho_matrix, tex_pkid);
}
extern void RICO_prim_draw_quad(const struct quad *quad,
                                const struct vec4 *color)
{
    prim_draw_quad(quad, color, &MAT4_IDENT, &cam_player.view_matrix,
                   cam_player.proj_matrix, 0);
}
extern void RICO_prim_draw_quad_xform(const struct quad *quad,
                                      const struct vec4 *color,
                                      const struct mat4 *xform)
{
    prim_draw_quad(quad, color, xform, &cam_player.view_matrix,
                   cam_player.proj_matrix, 0);
}
static void prim_draw_quad(const struct quad *quad, const struct vec4 *color,
                           const struct mat4 *xform, const struct mat4 *view,
                           const struct mat4 *proj, pkid tex_pkid)
{
    enum toolbar_icon {
        TOOLBAR_SELECT,
        TOOLBAR_TRANSLATE,
        TOOLBAR_ROTATE,
        TOOLBAR_SCALE,
        TOOLBAR_MESH,
        TOOLBAR_TEXTURE,
        TOOLBAR_NEW,
        TOOLBAR_COPY,
        TOOLBAR_DELETE,
        TOOLBAR_UNDO,
        TOOLBAR_REDO,
        TOOLBAR_SAVE,
        TOOLBAR_EXIT,
    };

    enum toolbar_icon icon = TOOLBAR_TRANSLATE;

    struct prim_vertex verts[4] = { 0 };
    verts[0].pos = quad->verts[0];
    verts[1].pos = quad->verts[1];
    verts[2].pos = quad->verts[2];
    verts[3].pos = quad->verts[3];

    float x0 = 1.0f / 16.0f * (float)icon;
    float y0 = 0.0f;
    float x1 = 1.0f / 16.0f * (float)(icon + 1);
    float y1 = 1.0f;
    verts[0].uv = VEC2F(x0, y0);
    verts[1].uv = VEC2F(x0, y1);
    verts[2].uv = VEC2F(x1, y0);
    verts[3].uv = VEC2F(x1, y1);

    RICO_ASSERT(prog_prim->program.gl_id);
    glUseProgram(prog_prim->program.gl_id);
    glUniformMatrix4fv(prog_prim->vert.proj, 1, GL_TRUE, proj->a);
    glUniformMatrix4fv(prog_prim->vert.view, 1, GL_TRUE, view->a);
    glUniformMatrix4fv(prog_prim->vert.model, 1, GL_TRUE, xform->a);

    RICO_ASSERT(prim_quad_vao);
    RICO_ASSERT(prim_quad_vbo);
    glBindVertexArray(prim_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, prim_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(quad->verts), quad->verts, GL_STATIC_DRAW);

    texture_bind(tex_pkid, GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glEnable(GL_CULL_FACE);
    texture_unbind(tex_pkid, GL_TEXTURE_2D);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Clean up
    glUseProgram(0);
}
extern void RICO_prim_draw_plane(const struct vec3 *n,
                                 const struct vec4 *color)
{
    RICO_prim_draw_plane_xform(n, color, &MAT4_IDENT);
}
extern void RICO_prim_draw_plane_xform(const struct vec3 *n,
                                       const struct vec4 *color,
                                       const struct mat4 *xform)
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

    RICO_prim_draw_quad_xform(&quad, color, xform);
}
extern void RICO_prim_draw_bbox(const struct RICO_bbox *bbox,
                                const struct vec4 *color)
{
    RICO_prim_draw_bbox_xform(bbox, color, &MAT4_IDENT);
}
extern void RICO_prim_draw_bbox_xform(const struct RICO_bbox *bbox,
                                      const struct vec4 *color,
                                      const struct mat4 *xform)
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

    RICO_ASSERT(prog_prim->program.gl_id);
    glUseProgram(prog_prim->program.gl_id);

    glUniformMatrix4fv(prog_prim->vert.proj, 1, GL_TRUE, cam_player.proj_matrix->a);
    glUniformMatrix4fv(prog_prim->vert.view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(prog_prim->vert.model, 1, GL_TRUE, xform->a);

    //glUniform4fv(prog_prim->col, 1, (const GLfloat *)color);
    // TODO: Bind texture

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
    RICO_prim_draw_obb_xform(obb, color, &MAT4_IDENT);
}
extern void RICO_prim_draw_obb_xform(const struct RICO_obb *obb,
                                     const struct vec4 *color,
                                     const struct mat4 *xform)
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
    RICO_prim_draw_line_xform(&obb->c, &axis0, &COLOR_RED, xform);
    RICO_prim_draw_line_xform(&obb->c, &axis1, &COLOR_GREEN, xform);
    RICO_prim_draw_line_xform(&obb->c, &axis2, &COLOR_BLUE, xform);

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

    RICO_ASSERT(prog_prim->program.gl_id);
    glUseProgram(prog_prim->program.gl_id);

    glUniformMatrix4fv(prog_prim->vert.proj, 1, GL_TRUE, cam_player.proj_matrix->a);
    glUniformMatrix4fv(prog_prim->vert.view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(prog_prim->vert.model, 1, GL_TRUE, xform->a);

    //glUniform4fv(prog_prim->col, 1, (const GLfloat *)color);
    // TODO: Bind texture

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
    RICO_prim_draw_sphere_xform(sphere, color, &MAT4_IDENT);
}
extern void RICO_prim_draw_sphere_xform(const struct sphere *sphere,
                                        const struct vec4 *color,
                                        const struct mat4 *xform)
{
    if (cam_player.fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // TODO: Test these!
#if 1
    struct mat4 model_matrix = *xform;
    mat4_translate(&model_matrix, &sphere->orig);
    mat4_scalef(&model_matrix, sphere->radius);
#else
    struct mat4 model_matrix = MAT4_IDENT;
    mat4_translate(&model_matrix, &sphere->orig);
    mat4_scalef(&model_matrix, sphere->radius);
    mat4_mul(&model_matrix, xform);
#endif

    RICO_ASSERT(prog_prim->program.gl_id);
    glUseProgram(prog_prim->program.gl_id);

    glUniformMatrix4fv(prog_prim->vert.proj, 1, GL_TRUE, cam_player.proj_matrix->a);
    glUniformMatrix4fv(prog_prim->vert.view, 1, GL_TRUE, cam_player.view_matrix.a);
    glUniformMatrix4fv(prog_prim->vert.model, 1, GL_TRUE, model_matrix.a);
    
    //glUniform4fv(prog_prim->col, 1, (const GLfloat *)color);
    // TODO: Bind texture
    
    // TODO: Render spheres with primitive shader rather than PBR shader? Need
    //       to store vertex data as prim_vertex instead of pbr_vertex.
    glBindVertexArray(mesh_vao(MESH_DEFAULT_SPHERE));
    mesh_render(MESH_DEFAULT_SPHERE);
    glBindVertexArray(0);

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
