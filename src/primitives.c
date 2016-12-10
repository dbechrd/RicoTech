#include "primitives.h"
#include "camera.h"
#include "program.h"
#include <GL/gl3w.h>

const char *rico_prim_string[] = {
    RICO_PRIMITIVES(GEN_STRING)
};

static GLuint vaos[PRIM_COUNT];
static GLuint vbos[PRIM_COUNT][VBO_COUNT];

static struct program_primitive *program;

static int prim_init_gl(enum rico_prim prim);

int prim_init(enum rico_prim prim)
{
    enum rico_error err = make_program_primitive(&program);
    if (err) return err;

    return prim_init_gl(prim);
}

static int prim_init_gl(enum rico_prim prim)
{
    //--------------------------------------------------------------------------
    // Generate VAO and buffers
    //--------------------------------------------------------------------------
    glGenVertexArrays(1, &vaos[prim]);
    glBindVertexArray(vaos[prim]);

    int buffers = 99;
    switch (prim)
    {
    case PRIM_LINE:
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
    glVertexAttribPointer(RICO_SHADER_POS_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, pos));
    glEnableVertexAttribArray(RICO_SHADER_POS_LOC);

    glVertexAttribPointer(RICO_SHADER_COL_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, col));
    glEnableVertexAttribArray(RICO_SHADER_COL_LOC);

    // Clean up
    glBindVertexArray(0);
    return SUCCESS;
}

void prim_draw_line(const struct line *line, const struct camera *camera,
                    const struct mat4 *model_matrix, struct col4 color)
{
    if (camera->fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Set shader program
    glUseProgram(program->prog_id);

    // Transform
    UNUSED(model_matrix);
    glUniformMatrix4fv(program->u_proj, 1, GL_TRUE, camera->proj_matrix.a);
    glUniformMatrix4fv(program->u_view, 1, GL_TRUE, camera->view_matrix.a);
    glUniformMatrix4fv(program->u_model, 1, GL_TRUE, MAT4_IDENT.a);

    glUniform4f(program->u_col, color.r, color.g, color.b, color.a);

    // Draw
    glBindVertexArray(vaos[PRIM_LINE]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[PRIM_LINE][VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line->vertices), line->vertices,
                 GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINES, 0, 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    if (camera->fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, camera->fill_mode);
}

void prim_free()
{
    // TODO: Clean-up prim VAO / VBO? Will probably just keep them for life
    //       of the application for now.
}