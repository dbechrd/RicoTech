#include "util.h"
#include "geom.h"
#include "rico_mesh.h"
#include "rico_object.h"
#include <stdio.h>
#include <stdlib.h>

#define MESH_VERTICES_MAX 500

enum OBJ_LINE_TYPE {
    OBJ_IGNORE,
    OBJ_MESH,
    OBJ_VERTEX,
    OBJ_TEXCOORD,
    OBJ_NORMAL,
    OBJ_FACE,
};

struct OBJ_FACE {
    uint32 idx_pos;
    uint32 idx_tex;
    uint32 idx_normal;
};

enum OBJ_LINE_TYPE line_type(const char *line);
//bool load_mesh(const char *line, struct rico_mesh *mesh);

static char* strsep(char** stringp, const char* delim)
{
    char* start = *stringp;
    char* p;

    p = (start != NULL) ? strpbrk(start, delim) : NULL;

    if (p == NULL)
    {
        *stringp = NULL;
    }
    else
    {
        *p = '\0';
        *stringp = p + 1;
    }

    return start;
}

int load_obj_file(const char *filename, uint32 *_meshes, uint32 *_mesh_count)
{
    enum rico_error err = SUCCESS;
    int length;
    char *buffer = file_contents(filename, &length);
    char *buffer_ptr = buffer;
    char *tok = strsep(&buffer_ptr, "\n");

    // TODO: Colossal waste of memory here, hmmm.
    struct vec4 positions[MESH_VERTICES_MAX] = { 0 };
    struct tex2 texcoords[MESH_VERTICES_MAX] = { 0 };
    struct vec4 normals[MESH_VERTICES_MAX] = { 0 };
    int idx_pos = 0;
    int idx_tex = 0;
    int idx_normal = 0;

    char *name = NULL;
    struct mesh_vertex vertices[MESH_VERTICES_MAX] = { 0 };
    GLuint elements[MESH_VERTICES_MAX];
    int idx_vertex = 0;
    int idx_mesh = 0;

    for (int i = 0; i < MESH_VERTICES_MAX; i++)
    {
        elements[i] = i;
    }

    while (tok != NULL)
    {
        if (str_starts_with(tok, "o "))
        {
            name = tok + 2;
        }
        else if (str_starts_with(tok, "v "))
        {
            char *pos = tok + 2;
            positions[idx_pos].x = strtof(pos, &pos);
            positions[idx_pos].y = strtof(pos, &pos);
            positions[idx_pos].z = strtof(pos, &pos);
            positions[idx_pos].w = 1.0f;
            idx_pos++;
        }
        else if (str_starts_with(tok, "vt "))
        {
            char *pos = tok + 3;
            texcoords[idx_tex].u = strtof(pos, &pos);
            texcoords[idx_tex].v = strtof(pos, &pos);
            idx_tex++;
        }
        else if (str_starts_with(tok, "vn "))
        {
            char *pos = tok + 3;
            normals[idx_normal].x = strtof(pos, &pos);
            normals[idx_normal].y = strtof(pos, &pos);
            normals[idx_normal].z = strtof(pos, &pos);
            normals[idx_normal].w = 0.0f;
            idx_normal++;
        }
        else if (str_starts_with(tok, "f "))
        {
            char *tok_ptr = tok + 2;
            char *vert = strsep(&tok_ptr, " ");
            while (vert != NULL)
            {
                long vert_pos = strtol(vert, &vert, 0);
                vert++;
                long vert_tex = strtol(vert, &vert, 0);
                vert++;
                //long vert_normal = strtol(vert, &vert);
                //vert++;
                vertices[idx_vertex].pos = positions[vert_pos - 1];
                vertices[idx_vertex].uv = texcoords[vert_tex - 1];
                //vertices[idx_vertex].normal = normals[vert_normal - 1];
                idx_vertex++;
                vert = strsep(&tok_ptr, " ");
            }
        }
        else if (str_starts_with(tok, "oe"))
        {
            UNUSED(normals);

            // TODO: Get program and texture from somewhere
            err = mesh_load(name, idx_vertex, vertices, idx_vertex, elements,
                            GL_STATIC_DRAW, &_meshes[idx_mesh]);
            if (err) goto cleanup;

            name = NULL;
            idx_vertex = 0;
            idx_mesh++;
        }
        tok = strsep(&buffer_ptr, "\n");
    }

    *_mesh_count = idx_mesh;
    printf("Loaded %s\n", filename);

cleanup:
    free(buffer);
    return err;
}

//enum OBJ_LINE_TYPE line_type(const char *line)
//{
//    if (str_starts_with(line, "o "))
//        return OBJ_OBJECT;
//    else if (str_starts_with(line, "v "))
//        return OBJ_VERTEX;
//    else if (str_starts_with(line, "vt "))
//        return OBJ_TEXCOORD;
//    else if (str_starts_with(line, "vn "))
//        return OBJ_NORMAL;
//    else if (str_starts_with(line, "f "))
//        return OBJ_FACE;
//    else
//        return OBJ_IGNORE;
//}

// bool load_mesh(const char *line, struct rico_mesh *mesh)
// {
//     static const char *prefix = "o ";
//     if (!str_starts_with(line, prefix))
//         return false;

//     line += strlen(prefix);

//     uid_init(line, &mesh->uid);
//     return line;
// }