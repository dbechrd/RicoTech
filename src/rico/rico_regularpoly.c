static struct program_primitive *regularpoly_program = NULL;

static int init_regularpoly_program()
{
    enum RICO_error err = make_program_primitive(&regularpoly_program);
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

    double delta_angle = M_2PI / (double)poly->count;
    double angle = 0;

    for (unsigned int i = 0; i < poly->count; i++)
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(poly->vertices[0])*poly->count,
                 poly->vertices, GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, poly_elements);

    //(GLuint index, GLint size, GLenum type,
    // GLboolean normalized, GLsizei stride, const void *pointer);
    glVertexAttribPointer(regularpoly_program->prog_id, 4, GL_FLOAT,
                          GL_FALSE, 0, 0);
    glEnableVertexAttribArray(regularpoly_program->attrs.position);

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
    poly->count = vertex_count;

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
    RICO_ASSERT(regularpoly_program->prog_id);

    if (poly->dirty_vao)
    {
        fprintf(stderr, "regularpoly: Rebuilding dirty VAO at render time");
        rebuild_vao(poly);
    }

    RICO_ASSERT(regularpoly_program->prog_id);
    glUseProgram(regularpoly_program->prog_id);
    glBindVertexArray(poly->vao);

    //(GLenum mode, GLint first, GLsizei count)
    glDrawArrays(
        GL_TRIANGLE_FAN,
        0,
        poly->count
    );

    glBindVertexArray(0);
}