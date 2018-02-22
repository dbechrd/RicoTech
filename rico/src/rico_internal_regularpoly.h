#ifndef RICO_INTERNAL_REGULARPOLY_H
#define RICO_INTERNAL_REGULARPOLY_H

struct regularpoly
{
    struct vec3 pos;
    GLfloat radius;

    struct vec3 *vertices;
    unsigned int count;
    GLuint vao;
    bool dirty_vao;
};

int init_regularpoly_program();
struct regularpoly *make_regularpoly(struct vec3 center, GLfloat radius,
                                unsigned int vertex_count);
static void free_regularpoly(struct regularpoly *);
static void set_regularpoly_pos(struct regularpoly *, GLfloat x, GLfloat y, GLfloat z,
                         bool rebuild);
static void render_regularpoly(struct regularpoly *);

#endif