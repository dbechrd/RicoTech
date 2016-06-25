#include "mesh.h"
#include <stdlib.h>

struct mesh *make_mesh(const struct mesh_vertex *vertex_data,
                       GLsizei vertex_count,
                       const GLushort *element_data,
                       GLsizei element_count,
                       GLenum hint)
{
    struct mesh *mesh = malloc(sizeof(struct mesh));

    glGenBuffers(1, &mesh->vertex_buffer);
    glGenBuffers(1, &mesh->element_buffer);
    mesh->element_count = element_count;

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertex_count * sizeof(struct mesh_vertex),
        vertex_data,
        hint
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        element_count * sizeof(GLushort),
        element_data,
        GL_STATIC_DRAW
    );

    return mesh;
}

void free_mesh(struct mesh *mesh)
{
    free(mesh);
    mesh = NULL;
}

void update_mesh(struct mesh *mesh)
{
    (void)mesh;
    //TODO: Animate the mesh.
}

void render_mesh(struct mesh *mesh)
{
    (void)mesh;
    /*glBindTexture(GL_TEXTURE_2D, mesh->texture);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glVertexAttribPointer(
        g_resources.flag_program.attributes.position,
        3, GL_FLOAT, GL_FALSE, sizeof(struct meshVertex),
        (void*)offsetof(struct meshVertex, position)
        );
    glVertexAttribPointer(
        g_resources.flag_program.attributes.normal,
        3, GL_FLOAT, GL_FALSE, sizeof(struct meshVertex),
        (void*)offsetof(struct meshVertex, normal)
        );
    glVertexAttribPointer(
        g_resources.flag_program.attributes.texcoord,
        2, GL_FLOAT, GL_FALSE, sizeof(struct meshVertex),
        (void*)offsetof(struct meshVertex, texcoord)
        );
    glVertexAttribPointer(
        g_resources.flag_program.attributes.shininess,
        1, GL_FLOAT, GL_FALSE, sizeof(struct meshVertex),
        (void*)offsetof(struct meshVertex, shininess)
        );
    glVertexAttribPointer(
        g_resources.flag_program.attributes.specular,
        4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct meshVertex),
        (void*)offsetof(struct meshVertex, specular)
        );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);
    glDrawElements(
        GL_TRIANGLES,
        mesh->element_count,
        GL_UNSIGNED_SHORT,
        (void*)0
    );*/
}
