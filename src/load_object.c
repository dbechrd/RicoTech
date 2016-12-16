#include "util.h"
#include "geom.h"
#include "rico_mesh.h"
#include "rico_object.h"
#include <stdio.h>
#include <stdlib.h>

//#define TINYOBJ_LOADER_C_IMPLEMENTATION
//#include "tinyobjloader.h"

#define MESH_VERTICES_MAX 5000000

enum OBJ_LINE_TYPE {
    OBJ_IGNORE,
    OBJ_MESH,
    OBJ_VERTEX,
    OBJ_TEXCOORD,
    OBJ_NORMAL,
    OBJ_FACE,
};

struct OBJ_FACE {
    u32 idx_pos;
    u32 idx_tex;
    u32 idx_normal;
};

enum OBJ_LINE_TYPE line_type(const char *line);
//bool load_mesh(const char *line, struct rico_mesh *mesh);

static inline char *strsep(char **stringp, const char *delim)
{
    char *start = *stringp;
    char *p;

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

static inline long fast_atol(const char *str)
{
    if (!str) return 0;

    long val = 0;
    while(*str) {
        val = val*10 + (*str++ - '0');
    }
    return val;
}

int load_obj_file(const char *filename, u32 *_meshes, u32 *_mesh_count)
{
    enum rico_error err = SUCCESS;
    int length;
    char *buffer;
    char *tok;

    err = file_contents(filename, &length, &buffer);
    if (err) goto cleanup;

    // TODO: Colossal waste of memory here, hmmm.
    struct vec3 *positions = calloc(MESH_VERTICES_MAX, sizeof(*positions));
    struct tex2 *texcoords = calloc(MESH_VERTICES_MAX, sizeof(*texcoords));
    struct vec3 *normals = calloc(MESH_VERTICES_MAX, sizeof(*normals));
    UNUSED(normals);

    //struct vec3 positions[MESH_VERTICES_MAX] = { 0 };
    //struct tex2 texcoords[MESH_VERTICES_MAX] = { 0 };
    //struct vec3 normals[MESH_VERTICES_MAX] = { 0 };
    int idx_pos = 0;
    int idx_tex = 0;
    int idx_normal = 0;

    char *name = NULL;

    struct mesh_vertex *vertices = calloc(MESH_VERTICES_MAX, sizeof(*vertices));
    GLuint *elements = calloc(MESH_VERTICES_MAX, sizeof(*elements));

    //struct mesh_vertex vertices[MESH_VERTICES_MAX] = { 0 };
    //GLuint elements[MESH_VERTICES_MAX];

    int idx_vertex = 0;
    int idx_element = 0;
    int idx_mesh = 0;

    // for (int i = 0; i < MESH_VERTICES_MAX; i++)
    // {
    //     elements[i] = i;
    // }

    long vert_pos = 0;
    long vert_tex = 0;
    long vert_normal = 0;

    char *buffer_ptr = buffer;
    for(;;)
    {
        tok = strsep(&buffer_ptr, "\r\n");
        if (!tok) break;

        // New object
        if (str_starts_with(tok, "o "))
        {
            if (idx_vertex > 0)
            {
                err = mesh_load(name, idx_vertex, vertices, idx_element,
                                elements, GL_STATIC_DRAW,
                                &_meshes[*_mesh_count + idx_mesh]);
                if (err) goto cleanup;
                idx_mesh++;
            }

            idx_vertex = 0;
            idx_element = 0;
            name = tok + 2;
        }
        else if (str_starts_with(tok, "v "))
        {
            //char *pos = tok + 2;
            tok += 2;
            positions[idx_pos].x = strtof(tok, &tok);
            positions[idx_pos].y = strtof(tok, &tok);
            positions[idx_pos].z = strtof(tok, &tok);
            if (++idx_pos >= MESH_VERTICES_MAX)
            {
                err = RICO_ERROR(ERR_OBJ_TOO_MANY_VERTS);
                goto cleanup;
            }
        }
        else if (str_starts_with(tok, "vt "))
        {
            //char *pos = tok + 3;
            tok += 3;
            texcoords[idx_tex].u = strtof(tok, &tok);
            texcoords[idx_tex].v = strtof(tok, &tok);
            if (++idx_tex >= MESH_VERTICES_MAX)
            {
                err = RICO_ERROR(ERR_OBJ_TOO_MANY_VERTS);
                goto cleanup;
            }
        }
        else if (str_starts_with(tok, "vn "))
        {
            //char *pos = tok + 3;
            tok += 3;
            normals[idx_normal].x = strtof(tok, &tok);
            normals[idx_normal].y = strtof(tok, &tok);
            normals[idx_normal].z = strtof(tok, &tok);
            if (++idx_normal >= MESH_VERTICES_MAX)
            {
                err = RICO_ERROR(ERR_OBJ_TOO_MANY_VERTS);
                goto cleanup;
            }
        }
        else if (str_starts_with(tok, "f "))
        {
            char *tok_ptr = tok + 2;
            char *vert = strsep(&tok_ptr, " ");
            while (vert != NULL)
            {
                vert_pos = fast_atol(strsep(&vert, "/"));
                vert_tex = fast_atol(strsep(&vert, "/"));
                vert_normal = fast_atol(strsep(&vert, "/"));

                vertices[idx_vertex].col = COLOR_WHITE;
                if (vert_pos > 0)
                    vertices[idx_vertex].pos = positions[vert_pos - 1];
                if (vert_tex > 0)
                    vertices[idx_vertex].uv = texcoords[vert_tex - 1];
                UNUSED(vert_normal);
                //if (vert_norm > 0)
                //    vertices[idx_vertex].normal = normals[vert_normal - 1];
                if (++idx_vertex >= MESH_VERTICES_MAX)
                {
                    err = RICO_ERROR(ERR_OBJ_TOO_MANY_VERTS);
                    goto cleanup;
                }

                elements[idx_element] = idx_vertex - 1;
                if (++idx_element >= MESH_VERTICES_MAX)
                {
                    err = RICO_ERROR(ERR_OBJ_TOO_MANY_VERTS);
                    goto cleanup;
                }

                vert = strsep(&tok_ptr, " ");
            }
        }
    }

    if (idx_vertex > 0)
    {
        err = mesh_load(name, idx_vertex, vertices, idx_element, elements,
                        GL_STATIC_DRAW, &_meshes[*_mesh_count + idx_mesh]);
        if (err) goto cleanup;
        idx_mesh++;
    }

    *_mesh_count += idx_mesh;
    printf("Loaded %s\n", filename);

cleanup:
    free(buffer);
    free(positions);
    free(texcoords);
    free(normals);
    free(vertices);
    free(elements);
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