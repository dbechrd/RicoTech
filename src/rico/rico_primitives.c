static GLuint prim_line_vao;
static GLuint prim_line_vbo;

static GLuint prim_quad_vao;
static GLuint prim_quad_vbo;

static GLuint prim_bbox_vao;
static GLuint prim_bbox_vbo[RIC_VBO_COUNT];

static int prim_init()
{
    enum ric_error err;

    err = make_program_primitive(&global_prog_primitive);
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

    RICO_ASSERT(global_prog_primitive->program.gl_id);
    glUseProgram(global_prog_primitive->program.gl_id);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.proj, 1, GL_TRUE, proj->a);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.view, 1, GL_TRUE, view->a);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.model, 1, GL_TRUE, xform->a);
    glUniform4fv(global_prog_primitive->locations.frag.color, 1, (const GLfloat *)color);
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
extern void RICO_prim_draw_sprite(const struct rect *rect,
                                  const struct RICO_sprite *sprite,
                                  const struct vec4 *color)
{
    // Triangle strip CCW winding order:
    // (0,0) (0,1) (1,0) (1,1)
    float x0 = X_TO_NDC(rect->x);
    float y0 = Y_TO_NDC(rect->y);
    float x1 = SCREEN_X(rect->x + rect->w);
    float y1 = SCREEN_Y(rect->y + rect->h);
    const float ortho_z = -1.0f;

    struct prim_vertex verts[4] = { 0 };
    verts[0].pos = VEC3(x0, y0, ortho_z);
    verts[1].pos = VEC3(x0, y1, ortho_z);
    verts[2].pos = VEC3(x1, y0, ortho_z);
    verts[3].pos = VEC3(x1, y1, ortho_z);

    float u0 = sprite->uvs[0].u;
    float v0 = sprite->uvs[0].v;
    float u1 = sprite->uvs[1].u;
    float v1 = sprite->uvs[1].v;
    verts[0].uv = VEC2(u0, v0);
    verts[1].uv = VEC2(u0, v1);
    verts[2].uv = VEC2(u1, v0);
    verts[3].uv = VEC2(u1, v1);

    prim_draw_quad(ARRAY_COUNT(verts), verts, color, &MAT4_IDENT, &MAT4_IDENT,
                   &cam_player.ortho_matrix, sprite->sheet->tex_id);
}
extern void RICO_prim_draw_rect_tex(const struct rect *rect,
                                    const struct vec4 *color, pkid tex_id)
{
    // Triangle strip CCW winding order:
    // (0,0) (0,1) (1,0) (1,1)
    float x0 = X_TO_NDC(rect->x);
    float y0 = Y_TO_NDC(rect->y);
    float x1 = SCREEN_X(rect->x + rect->w);
    float y1 = SCREEN_Y(rect->y + rect->h);
    const float ortho_z = -1.0f;

    struct prim_vertex verts[4] = { 0 };
    verts[0].pos = VEC3(x0, y0, ortho_z);
    verts[1].pos = VEC3(x0, y1, ortho_z);
    verts[2].pos = VEC3(x1, y0, ortho_z);
    verts[3].pos = VEC3(x1, y1, ortho_z);

    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
    verts[0].uv = VEC2(u0, v0);
    verts[1].uv = VEC2(u0, v1);
    verts[2].uv = VEC2(u1, v0);
    verts[3].uv = VEC2(u1, v1);

    prim_draw_quad(ARRAY_COUNT(verts), verts, color, &MAT4_IDENT, &MAT4_IDENT,
                   &cam_player.ortho_matrix, tex_id);
}
extern void RICO_prim_draw_quad(const struct quad *quad,
                                const struct vec4 *color)
{
    RICO_prim_draw_quad_xform(quad, color, &MAT4_IDENT);
}
extern void RICO_prim_draw_quad_xform(const struct quad *quad,
                                      const struct vec4 *color,
                                      const struct mat4 *xform)
{
    struct prim_vertex verts[4] = { 0 };
    verts[0].pos = quad->verts[0];
    verts[1].pos = quad->verts[1];
    verts[2].pos = quad->verts[2];
    verts[3].pos = quad->verts[3];
    prim_draw_quad(ARRAY_COUNT(verts), verts, color, xform,
                   &cam_player.view_matrix, cam_player.proj_matrix, 0);
}
static void prim_draw_quad(u32 vertex_count, const struct prim_vertex *vertices,
                           const struct vec4 *color, const struct mat4 *xform,
                           const struct mat4 *view, const struct mat4 *proj,
                           pkid tex_id)
{
    RICO_ASSERT(global_prog_primitive->program.gl_id);
    glUseProgram(global_prog_primitive->program.gl_id);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.proj, 1, GL_TRUE, proj->a);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.view, 1, GL_TRUE, view->a);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.model, 1, GL_TRUE, xform->a);
    glUniform4fv(global_prog_primitive->locations.frag.color, 1, (const GLfloat *)color);

    RICO_ASSERT(prim_quad_vao);
    RICO_ASSERT(prim_quad_vbo);
    glBindVertexArray(prim_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, prim_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(struct prim_vertex),
                 vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (tex_id) texture_bind(tex_id, GL_TEXTURE0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    if (tex_id) texture_unbind(tex_id, GL_TEXTURE0);

    glBindVertexArray(0);
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
extern void RICO_prim_draw_bbox(const struct RICO_aabb *aabb,
                                const struct vec4 *color)
{
    RICO_prim_draw_bbox_xform(aabb, color, &MAT4_IDENT);
}
extern void RICO_prim_draw_bbox_xform(const struct RICO_aabb *aabb,
                                      const struct vec4 *color,
                                      const struct mat4 *xform)
{
    struct vec3 p0 = aabb->c;
    v3_sub(&p0, &aabb->e);
    struct vec3 p1 = aabb->c;
    v3_add(&p1, &aabb->e);

    GLfloat vertices[] = {
        p0.x, p0.y, p0.z,
        p1.x, p0.y, p0.z,
        p1.x, p1.y, p0.z,
        p0.x, p1.y, p0.z,
        p0.x, p0.y, p1.z,
        p1.x, p0.y, p1.z,
        p1.x, p1.y, p1.z,
        p0.x, p1.y, p1.z,
    };

    RICO_ASSERT(global_prog_primitive->program.gl_id);
    glUseProgram(global_prog_primitive->program.gl_id);

    glUniformMatrix4fv(global_prog_primitive->locations.vert.proj, 1, GL_TRUE,
                       cam_player.proj_matrix->a);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.view, 1, GL_TRUE,
                       cam_player.view_matrix.a);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.model, 1, GL_TRUE, xform->a);
    glUniform4fv(global_prog_primitive->locations.frag.color, 1, (const GLfloat *)color);
    // TODO: Bind texture

    RICO_ASSERT(prim_bbox_vao);
    RICO_ASSERT(prim_bbox_vbo[0]);
    RICO_ASSERT(prim_bbox_vbo[1]);
    glBindVertexArray(prim_bbox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, prim_bbox_vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
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

    RICO_ASSERT(global_prog_primitive->program.gl_id);
    glUseProgram(global_prog_primitive->program.gl_id);

    glUniformMatrix4fv(global_prog_primitive->locations.vert.proj, 1, GL_TRUE,
                       cam_player.proj_matrix->a);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.view, 1, GL_TRUE,
                       cam_player.view_matrix.a);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.model, 1, GL_TRUE, xform->a);
    glUniform4fv(global_prog_primitive->locations.frag.color, 1, (const GLfloat *)color);
    // TODO: Bind texture

    RICO_ASSERT(prim_bbox_vao);
    RICO_ASSERT(prim_bbox_vbo[0]);
    RICO_ASSERT(prim_bbox_vbo[1]);
    glBindVertexArray(prim_bbox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, prim_bbox_vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
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

    RICO_ASSERT(global_prog_primitive->program.gl_id);
    glUseProgram(global_prog_primitive->program.gl_id);

    glUniformMatrix4fv(global_prog_primitive->locations.vert.proj, 1, GL_TRUE,
                       cam_player.proj_matrix->a);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.view, 1, GL_TRUE,
                       cam_player.view_matrix.a);
    glUniformMatrix4fv(global_prog_primitive->locations.vert.model, 1, GL_TRUE,
                       model_matrix.a);
    glUniform4fv(global_prog_primitive->locations.frag.color, 1, (const GLfloat *)color);
    // TODO: Bind texture

    // TODO: Render spheres with primitive shader rather than PBR shader? Need
    //       to store vertex data as prim_vertex instead of pbr_vertex.
    glBindVertexArray(mesh_vao(global_default_mesh_sphere));
    mesh_render(global_default_mesh_sphere);
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

#if 0
static struct program_primitive *regularpoly_program = NULL;

static int init_regularpoly_program()
{
    enum ric_error err = make_program_primitive(&regularpoly_program);
    if (err) {
        fprintf(stderr, "regularpoly: Failed to make program.\n");
    }

    return err;
}
static void rebuild_vao(struct regularpoly *poly)
{
    if (poly->vao)
    {
        glDeleteVertexArrays(1, &poly->vao);
    }

    RICO_ASSERT(regularpoly_program);

    double delta_angle = M_2PI / (double)poly->vertex_count;
    double angle = 0;

    for (unsigned int i = 0; i < poly->vertex_count; i++)
    {
        poly->vertices[i].x = (GLfloat)cos(angle) * poly->radius + poly->pos.x;
        poly->vertices[i].y = (GLfloat)sin(angle) * poly->radius + poly->pos.y;
        poly->vertices[i].z = poly->pos.z;
        angle += delta_angle;
    }

    glGenVertexArrays(1, &poly->vao);
    glBindVertexArray(poly->vao);

    GLuint poly_vbo;

    glGenBuffers(1, &poly_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, poly_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(poly->vertices[0])*poly->vertex_count,
                 poly->vertices, GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, poly_elements);

    //(GLuint index, GLint min_size, GLenum type,
    // GLboolean normalized, GLsizei stride, const void *pointer);
    glVertexAttribPointer(regularpoly_program->program.gl_id, 4, GL_FLOAT,
                          GL_FALSE, 0, 0);
    glEnableVertexAttribArray(regularpoly_program->locations.vert.attrs.position);

    glBindVertexArray(0);
    glDeleteBuffers(1, &poly_vbo);

    poly->dirty_vao = false;
}
static struct regularpoly *make_regularpoly(struct vec3 center, GLfloat radius,
                                            unsigned int vertex_count)
{
    struct regularpoly *poly = calloc(1, sizeof(struct regularpoly));
    if (!poly)
        return NULL;

    poly->pos = center;
    poly->radius = radius;

    poly->vertices = calloc(1, sizeof(struct vec3) * vertex_count);
    poly->vertex_count = vertex_count;

    poly->vao = 0;
    poly->dirty_vao = true;
    rebuild_vao(poly);

    return poly;
}
static void free_regularpoly(struct regularpoly *poly)
{
    free(poly->vertices);
    poly->vertices = NULL;

    free(poly);
    poly = NULL;
}
static void set_regularpoly_pos(struct regularpoly *poly, GLfloat x, GLfloat y,
                                GLfloat z, bool rebuild)
{
    poly->pos.x = x;
    poly->pos.y = y;
    poly->pos.z = z;

    if (rebuild)
        rebuild_vao(poly);
    else
        poly->dirty_vao = true;

}
static void render_regularpoly(struct regularpoly *poly)
{
    RICO_ASSERT(regularpoly_program->program.gl_id);

    if (poly->dirty_vao)
    {
        fprintf(stderr, "regularpoly: Rebuilding dirty VAO at render time");
        rebuild_vao(poly);
    }

    RICO_ASSERT(regularpoly_program->program.gl_id);
    glUseProgram(regularpoly_program->program.gl_id);
    glBindVertexArray(poly->vao);

    //(GLenum mode, GLint first, GLsizei bucket_count)
    glDrawArrays(
        GL_TRIANGLE_FAN,
        0,
        poly->vertex_count
    );

    glBindVertexArray(0);
}
#endif