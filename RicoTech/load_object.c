#include "util.h"
#include "geom.h"
#include "rico_mesh.h"
#include "rico_obj.h"
#include <stdio.h>
#include <stdlib.h>

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
bool load_mesh(const char *line, struct rico_mesh *mesh);
bool load_mesh(const char *line, struct rico_mesh *mesh);
bool load_mesh(const char *line, struct rico_mesh *mesh);
bool load_mesh(const char *line, struct rico_mesh *mesh);
bool load_mesh(const char *line, struct rico_mesh *mesh);

char* strsep(char** stringp, const char* delim)
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

void load_objects(const char *filename, struct rico_mesh ***mesh_result, 
                  int *mesh_count)
{
    int length;
    char *buffer = file_contents(filename, &length);
    char *buffer_ptr = buffer;
    char *tok = strsep(&buffer_ptr, "\n");

    // TODO: Collossal waste of memory here, hmmm.
    struct vec4 positions[500] = { 0 };
    struct tex2 texcoords[500] = { 0 };
    struct vec4 normals[500] = { 0 };
    int idx_pos = 0;
    int idx_tex = 0;
    int idx_normal = 0;

    char *name = NULL;
    struct mesh_vertex vertices[500] = { 0 };
    GLuint elements[500] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
        50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
        60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
        70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
        90, 91, 92, 93, 94, 95, 96, 97, 98, 99
    };
    struct rico_mesh **meshes = calloc(100, sizeof(*meshes));
    int idx_vertex = 0;
    int idx_mesh = 0;

    while (tok != NULL)
    {
        if (str_starts_with(tok, "o "))
        {
            name = tok + strlen("o ");
        }
        else if (str_starts_with(tok, "v "))
        {
            char *pos = tok + strlen("v ");
            positions[idx_pos].x = strtof(pos, &pos);
            positions[idx_pos].y = strtof(pos, &pos);
            positions[idx_pos].z = strtof(pos, &pos);
            positions[idx_pos].w = 1.0f;
            idx_pos++;
        }
        else if (str_starts_with(tok, "vt "))
        {
            char *pos = tok + strlen("vt ");
            texcoords[idx_tex].u = strtof(pos, &pos);
            texcoords[idx_tex].v = strtof(pos, &pos);
            idx_tex++;
        }
        else if (str_starts_with(tok, "vn "))
        {
            char *pos = tok + strlen("vn ");
            normals[idx_normal].x = strtof(pos, &pos);
            normals[idx_normal].y = strtof(pos, &pos);
            normals[idx_normal].z = strtof(pos, &pos);
            normals[idx_normal].w = 0.0f;
            idx_normal++;
        }
        else if (str_starts_with(tok, "f "))
        {
            char *tok_ptr = tok + strlen("f ");
            char *vert = strsep(&tok_ptr, " ");
            while (vert != NULL)
            {
                long vert_pos = strtol(vert, &vert, 0);
                vert++;
                long vert_tex = strtol(vert, &vert, 0);
                vert++;
                //int vert_normal = strtoi(idx, &idx);
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
            // TODO: Get program and texture from somewhere
            meshes[idx_mesh] = make_mesh(name, NULL, NULL, vertices, idx_vertex,
                                         elements, idx_vertex, GL_STATIC_DRAW);

            name = NULL;
            idx_vertex = 0;
            idx_mesh++;
        }
        tok = strsep(&buffer_ptr, "\n");
    }

    /*while (tok != NULL)
    {
        printf("> %s\n", tok);
        tok = strtok(NULL, "\n");
    }*/

    free(buffer);

    printf("Loaded %s\n", filename);
    *mesh_result = meshes;
    *mesh_count = idx_mesh;
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

bool load_mesh(const char *line, struct rico_mesh *mesh)
{
    static const char *prefix = "o ";
    if (!str_starts_with(line, prefix))
        return false;

    line += strlen(prefix);

    uid_init(line, &mesh->uid);
    return line;
}