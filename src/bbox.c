#include "bbox.h"
#include "const.h"
#include "geom.h"
#include "camera.h"
#include "rico_mesh.h"
#include "program.h"
#include "primitives.h"
#include <GL/gl3w.h>

static int init_gl(struct bbox *bbox);

int bbox_init(struct bbox *bbox, const char *name, struct vec3 p0,
              struct vec3 p1, struct col4 color)
{
    uid_init(&bbox->uid, RICO_UID_BBOX, name);
    bbox->p0 = p0;
    bbox->p1 = p1;
    bbox->color = color;
    bbox->wireframe = true;

    return init_gl(bbox);
}

int bbox_init_mesh(struct bbox *bbox, const char *name,
                   const struct mesh_vertex *verts, int count,
                   struct col4 color)
{
    struct vec3 p0 = (struct vec3) { 9999.0f, 9999.0f, 9999.0f };
    struct vec3 p1 = (struct vec3) { -9999.0f, -9999.0f, -9999.0f };

    // Find bounds of mesh
    for (int i = 0; i < count; ++i)
    {
        if (verts[i].pos.x < p0.x)
            p0.x = verts[i].pos.x;
        else if (verts[i].pos.x > p1.x)
            p1.x = verts[i].pos.x;

        if (verts[i].pos.y < p0.y)
            p0.y = verts[i].pos.y;
        else if (verts[i].pos.y > p1.y)
            p1.y = verts[i].pos.y;

        if (verts[i].pos.z < p0.z)
            p0.z = verts[i].pos.z;
        else if (verts[i].pos.z > p1.z)
            p1.z = verts[i].pos.z;
    }

    // Prevent infinitesimally small bounds
    p0.x -= EPSILON;
    p1.x += EPSILON;
    p0.y -= EPSILON;
    p1.y += EPSILON;
    p0.z -= EPSILON;
    p1.z += EPSILON;

    return bbox_init(bbox, name, p0, p1, color);
}

static int init_gl(struct bbox *bbox)
{
    enum rico_error err = make_program_primitive(&bbox->prog);
    if (err) return err;

    // Bbox vertices
    struct prim_vertex vertices[8] = {
        (struct prim_vertex) {
            (struct vec3) { bbox->p0.x, bbox->p0.y, bbox->p0.z },
            COLOR_BLACK
        },
        (struct prim_vertex) {
            (struct vec3) { bbox->p1.x, bbox->p0.y, bbox->p0.z },
            COLOR_RED
        },
        (struct prim_vertex) {
            (struct vec3) { bbox->p1.x, bbox->p1.y, bbox->p0.z },
            COLOR_YELLOW
        },
        (struct prim_vertex) {
            (struct vec3) { bbox->p0.x, bbox->p1.y, bbox->p0.z },
            COLOR_GREEN
        },
        (struct prim_vertex) {
            (struct vec3) { bbox->p0.x, bbox->p0.y, bbox->p1.z },
            COLOR_BLUE
        },
        (struct prim_vertex) {
            (struct vec3) { bbox->p1.x, bbox->p0.y, bbox->p1.z },
            COLOR_MAGENTA
        },
        (struct prim_vertex) {
            (struct vec3) { bbox->p1.x, bbox->p1.y, bbox->p1.z },
            COLOR_WHITE
        },
        (struct prim_vertex) {
            (struct vec3) { bbox->p0.x, bbox->p1.y, bbox->p1.z },
            COLOR_CYAN
        }
    };

    // Bbox faces
    static GLuint elements[36] = {
        0, 1, 2, 2, 3, 0,
        4, 0, 3, 3, 7, 4,
        5, 4, 7, 7, 6, 5,
        1, 5, 6, 6, 2, 1,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4
    };

    //--------------------------------------------------------------------------
    // Generate VAO and buffers
    //--------------------------------------------------------------------------
    glGenVertexArrays(1, &bbox->vao);
    glBindVertexArray(bbox->vao);
    glGenBuffers(2, bbox->vbos);

    //--------------------------------------------------------------------------
    // Vertex buffer
    //--------------------------------------------------------------------------
    glBindBuffer(GL_ARRAY_BUFFER, bbox->vbos[VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                 GL_STATIC_DRAW);

    //--------------------------------------------------------------------------
    // Element buffer
    //--------------------------------------------------------------------------
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bbox->vbos[VBO_ELEMENT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements,
                 GL_STATIC_DRAW);

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
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return SUCCESS;
}

void bbox_free_mesh(struct bbox *bbox)
{
    glDeleteBuffers(2, bbox->vbos);
    glDeleteVertexArrays(1, &bbox->vao);
}

void bbox_render(const struct bbox *box, const struct camera *camera,
                 const struct mat4 *model_matrix)
{
    bbox_render_color(box, camera, model_matrix, box->color);
}

void bbox_render_color(const struct bbox *box, const struct camera *camera,
                       const struct mat4 *model_matrix,
                       const struct col4 color)
{
    if (box->wireframe && camera->fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Set shader program
    glUseProgram(box->prog->prog_id);

    // Transform
    glUniformMatrix4fv(box->prog->u_proj, 1, GL_TRUE, camera->proj_matrix.a);
    glUniformMatrix4fv(box->prog->u_view, 1, GL_TRUE, camera->view_matrix.a);
    glUniformMatrix4fv(box->prog->u_model, 1, GL_TRUE, model_matrix->a);

    glUniform4f(box->prog->u_col, color.r, color.g, color.b, color.a);

    // Draw
    glBindVertexArray(box->vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    if (box->wireframe && camera->fill_mode != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, camera->fill_mode);
}

int bbox_serialize_0(const void *handle, const struct rico_file *file)
{
    const struct bbox *bbox = handle;
    fwrite(&bbox->p0,        sizeof(bbox->p0),        1, file->fs);
    fwrite(&bbox->p1,        sizeof(bbox->p1),        1, file->fs);
    fwrite(&bbox->color,     sizeof(bbox->color),     1, file->fs);
    fwrite(&bbox->wireframe, sizeof(bbox->wireframe), 1, file->fs);
    return SUCCESS;
}

int bbox_deserialize_0(void *_handle, const struct rico_file *file)
{
    enum rico_error err;
    struct bbox *bbox = _handle;

    fread(&bbox->p0,        sizeof(bbox->p0),        1, file->fs);
    fread(&bbox->p1,        sizeof(bbox->p1),        1, file->fs);
    fread(&bbox->color,     sizeof(bbox->color),     1, file->fs);
    fread(&bbox->wireframe, sizeof(bbox->wireframe), 1, file->fs);
    err = init_gl(bbox);
    return err;
}
