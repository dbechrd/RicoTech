#include "rico_object.h"
#include "const.h"
#include "rico_pool.h"
#include "camera.h"
#include "rico_cereal.h"
#include "program.h"
#include "rico_font.h"
#include "rico_collision.h"
#include "rico_material.h"
#include "rico_light.h"
#include "rico_state.h"
#include <malloc.h>

struct rico_object {
    struct rico_uid uid;
    enum rico_obj_type type;

    //TODO: Refactor into rico_transform
    //TODO: Animation
    struct vec3 trans;
    struct vec3 rot;
    struct vec3 scale;
    struct mat4 transform;
    struct mat4 transform_inverse;

    //TODO: Support multiple meshes
    u32 mesh;

    //TODO: Support multiple textures (per mesh?)
    u32 material;

    struct bbox bbox;
};

const char *rico_obj_type_string[] = {
    RICO_OBJ_TYPES(GEN_STRING)
};

u32 RICO_OBJECT_DEFAULT = 0;
static struct rico_pool *objects;

static void update_transform(struct rico_object *obj);

int rico_object_init(u32 pool_size)
{
    objects = calloc(1, sizeof(*objects));
    return pool_init("Objects", pool_size, sizeof(struct rico_object), 0,
                     objects);
}

int object_create(u32 *_handle, const char *name, enum rico_obj_type type,
                  u32 mesh, u32 material, const struct bbox *bbox,
                  bool serialize)
{
#ifdef RICO_DEBUG_OBJECT
    printf("[ obj][init] name=%s\n", name);
#endif

    enum rico_error err;
    *_handle = RICO_OBJECT_DEFAULT;

    err = pool_alloc(objects, _handle);
    if (err) return err;

    struct rico_object *obj = pool_read(objects, *_handle);

    uid_init(&obj->uid, RICO_UID_OBJECT, name, serialize);
    obj->type = type;
    if (type == OBJ_STRING_SCREEN)
        obj->scale = VEC3_SCALE_ASPECT;
    else
        obj->scale = VEC3_ONE;
    obj->mesh = mesh_request(mesh);
    obj->material = material_request(material);
    obj->bbox = (bbox != NULL) ? *bbox : *mesh_bbox(mesh);
    update_transform(obj);

    return err;
}

int object_copy(u32 *_handle, u32 handle, const char *name)
{
    enum rico_error err;
    struct rico_object *obj = pool_read(objects, handle);

    // Create new object with same mesh / texture
    err = object_create(_handle, name, obj->type, obj->mesh, obj->material,
                        NULL, true);
    if (err) return err;

    // Copy transform
    object_trans_set(*_handle, &obj->trans);
    object_rot_set(*_handle, &obj->rot);
    object_scale_set(*_handle, &obj->scale);

    return err;
}

void object_mesh_set(u32 handle, u32 mesh, const struct bbox *bbox)
{
    struct rico_object *obj = pool_read(objects, handle);
    mesh_free(obj->mesh);
    obj->mesh = mesh_request(mesh);
    obj->bbox = (bbox != NULL) ? *bbox : *mesh_bbox(mesh);
}

void object_material_set(u32 handle, u32 material)
{
    struct rico_object *obj = pool_read(objects, handle);
    material_free(obj->material);
    obj->material = material_request(material);
}

void object_free(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);

#ifdef RICO_DEBUG_INFO
    printf("[ obj][free] name=%s\n", obj->uid.name);
#endif

    mesh_free(obj->mesh);
    material_free(obj->material);

    obj->uid.uid = UID_NULL;
    pool_free(objects, handle);
}

void object_free_all()
{
    for (int i = objects->active - 1; i >= 0; --i)
    {
        object_free(objects->handles[i]);
    }
}

u32 object_next(u32 handle)
{
    return pool_next(objects, handle);
}

u32 object_prev(u32 handle)
{
    return pool_prev(objects, handle);
}

enum rico_obj_type object_type_get(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    return obj->type;
}

void object_select(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->bbox.wireframe = false;
}

void object_deselect(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->bbox.wireframe = true;
}

void object_trans(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    vec3_add(&obj->trans, v);
    update_transform(obj);
}

const struct vec3 *object_trans_get(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    return &obj->trans;
}

void object_trans_set(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->trans = *v;
    update_transform(obj);
}

void object_rot(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    vec3_add(&obj->rot, v);
    update_transform(obj);
}

void object_rot_set(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot = *v;
    update_transform(obj);
}

const struct vec3 *object_rot_get(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    return &obj->rot;
}

void object_rot_x(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.x += deg;
    update_transform(obj);
}

void object_rot_x_set(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.x = deg;
    update_transform(obj);
}

void object_rot_y(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.y += deg;
    update_transform(obj);
}

void object_rot_y_set(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.y = deg;
    update_transform(obj);
}

void object_rot_z(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.z += deg;
    update_transform(obj);
}

void object_rot_z_set(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.z = deg;
    update_transform(obj);
}

void object_scale(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    vec3_add(&obj->scale, v);
    update_transform(obj);
}

void object_scale_set(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->scale = *v;
    update_transform(obj);
}

const struct vec3 *object_scale_get(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    return &obj->scale;
}

static void update_transform(struct rico_object *obj)
{
    //HACK: Order of these operations might not always be the same.. should
    //      probably just store the transformation matrix directly rather than
    //      trying to figure out which order to do what. Unfortunately, doing
    //      this makes relative transformations very difficult. Maybe objects
    //      should have "edit mode" where the matrix is decomposed, then
    //      recomposed again when edit mode is ended?
    obj->transform = MAT4_IDENT;
    mat4_translate(&obj->transform, &obj->trans);
    mat4_rotx(&obj->transform, obj->rot.x);
    mat4_roty(&obj->transform, obj->rot.y);
    mat4_rotz(&obj->transform, obj->rot.z);
    mat4_scale(&obj->transform, &obj->scale);

    struct vec3 scale_inv;
    scale_inv.x = 1.0f / obj->scale.x;
    scale_inv.y = 1.0f / obj->scale.y;
    scale_inv.z = 1.0f / obj->scale.z;

    struct vec3 trans_inv = obj->trans;
    vec3_negate(&trans_inv);

    obj->transform_inverse = MAT4_IDENT;
    mat4_scale(&obj->transform_inverse, &scale_inv);
    mat4_rotz(&obj->transform_inverse, -obj->rot.z);
    mat4_roty(&obj->transform_inverse, -obj->rot.y);
    mat4_rotx(&obj->transform_inverse, -obj->rot.x);
    mat4_translate(&obj->transform_inverse, &trans_inv);

    //struct mat4 mm = obj->transform;
    //mat4_mul(&mm, &obj->transform_inverse);
    //RICO_ASSERT(mat4_equals(&mm, &MAT4_IDENT));
}

const struct mat4 *object_transform_get(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    return &obj->transform;
}

bool object_collide_ray(u32 handle, const struct ray *ray, float *_dist)
{
    struct rico_object *obj = pool_read(objects, handle);
    return collide_ray_obb(ray, &obj->bbox, &obj->transform,
                           &obj->transform_inverse, _dist);
}

u32 object_collide_ray_type(enum rico_obj_type type, const struct ray *ray,
                            u32 count, u32 *_handle, float *_dist, u32 *_first)
{
    u32 idx_collide = 0;
    float distance;
    float min_distance = Z_FAR; // Track closest object

    struct rico_object *obj;
    for (u32 i = 0; i < objects->active; ++i)
    {
        obj = pool_read(objects, objects->handles[i]);
        if (obj->type == type &&
            collide_ray_obb(ray, &obj->bbox, &obj->transform,
                            &obj->transform_inverse, &distance))
        {
            // Record object handle and distance
            _handle[idx_collide] = objects->handles[i];
            _dist[idx_collide] = distance;

            // If closest so far, update "first" index
            if (distance < min_distance)
            {
                min_distance = distance;
                *_first = idx_collide;
            }

            if (idx_collide++ == count)
                break;
        }
    }

    return idx_collide;
}

static void render(const struct rico_object *obj,
                   const struct camera *camera)
{
    if (obj->type == OBJ_STATIC)
    {
        glPolygonMode(GL_FRONT_AND_BACK, camera->fill_mode);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }


    // Bind material and render mesh
    material_bind(obj->material);
    mesh_render(obj->mesh);

    // Clean up
    material_unbind(obj->material);
}

void object_render_type(enum rico_obj_type type,
                        const struct program_default *prog,
                        const struct camera *camera)
{
    glUseProgram(prog->prog_id);

    // Model transform
    struct mat4 proj_matrix = camera->proj_matrix;
    struct mat4 view_matrix = camera->view_matrix;

    struct light_point light;
    light.color    = (struct vec3){ 1.0f, 0.9f, 0.6f };
    light.position = (struct vec3){ 0.0f, 3.0f, 0.0f };
    light.ambient  = (struct vec3){ 0.07f, 0.07f, 0.09f };
    light.kc = 1.0f;
    light.kl = 0.05f;
    light.kq = 0.001f;

    if (type == OBJ_STRING_SCREEN)
    {
        // TODO: Create dedicated shader for OBJ_STRING_SCREEN instead of using
        //       all of these hacks to disable scaling, projection, lighting...
        proj_matrix = MAT4_IDENT;
        view_matrix = MAT4_IDENT;

        light.color   = VEC3_ONE;
        light.ambient = VEC3_ONE;

        light.kc = 1.0f;
        light.kl = 0.0f;
        light.kq = 0.0f;
    }

    glUniformMatrix4fv(prog->u_proj, 1, GL_TRUE, proj_matrix.a);
    glUniformMatrix4fv(prog->u_view, 1, GL_TRUE, view_matrix.a);
    glUniform3fv(prog->u_view_pos, 1, (const GLfloat *)&camera->position);

    // Material textures
    // Note: We don't have to do this every time as long as we make sure
    //       the correct textures are bound before each draw to the texture
    //       index assumed when the program was initialized.
    glUniform1i(prog->u_material_diff, 0);
    glUniform1i(prog->u_material_spec, 1);

    // Lighting
    glUniform3fv(prog->u_light_position, 1, (const GLfloat *)&light.position);
    glUniform3fv(prog->u_light_ambient,  1, (const GLfloat *)&light.ambient);
    glUniform3fv(prog->u_light_color,    1, (const GLfloat *)&light.color);
    glUniform1f(prog->u_light_kc, light.kc);
    glUniform1f(prog->u_light_kl, light.kl);
    glUniform1f(prog->u_light_kq, light.kq);

    struct rico_object *obj;
    for (u32 i = 0; i < objects->active; ++i)
    {
        obj = pool_read(objects, objects->handles[i]);
        if (obj->type == type)
        {
            glUseProgram(prog->prog_id);

            ////////////////////////////////////////////////////////////////////
            // Set object-specific uniform values
            ////////////////////////////////////////////////////////////////////

            // UV-coord scale
            /* HACK: This only works when object is uniformly scaled on X/Y
                     plane. */
            /* TODO: UV scaling in general only works when object is uniformly
                     scaled. Maybe I should only allow textured objects to be
                     uniformly scaled? */
            if (type == OBJ_STRING_WORLD || type == OBJ_STRING_SCREEN)
            {
                glUniform2f(prog->u_scale_uv, 1.0f, 1.0f);
            }
            else
            {
                glUniform2f(prog->u_scale_uv, obj->scale.x, obj->scale.y);
            }

            // Model matrix
            glUniformMatrix4fv(prog->u_model, 1, GL_TRUE, obj->transform.a);

            // Model material shiny
            glUniform1f(prog->u_material_shiny,
                        material_shiny_get(obj->material));

            // Render object
            render(obj, camera);

            // TODO: Batch bounding boxes
            // Render bbox
            if (state_is_edit())
                bbox_render(&obj->bbox, &obj->transform);
        }
    }

    glUseProgram(0);
}

int object_print(u32 handle, enum rico_string_slot slot)
{
    enum rico_error err;

    // Print to screen
    char *buf = object_to_string(handle);
    err = string_init(rico_string_slot_string[slot], slot, 0, 26,
                      COLOR_GRAY_HIGHLIGHT, 0, RICO_FONT_DEFAULT, buf);
    if (err) goto cleanup;

cleanup:
    free(buf);
    return err;
}

char *object_to_string(u32 handle)
{
    char *buf = calloc(1, 256);

    if (!handle)
    {
        sprintf(buf,
            "    UID: %d\n" \
            "   Name: --\n" \
            "   Type: --\n" \
            "  Trans: --\n" \
            "    Rot: --\n" \
            "  Scale: --\n" \
            "   Mesh: --\n" \
            "Texture: --\n",
            UID_NULL);
    }
    else
    {
        struct rico_object *obj = pool_read(objects, handle);
        sprintf(buf,
            "     UID: %d\n"       \
            "    Name: %s\n"       \
            "    Type: %s\n"       \
            "   Trans: %f %f %f\n" \
            "     Rot: %f %f %f\n" \
            "   Scale: %f %f %f\n" \
            "    Mesh: %d\n"       \
            "Material: %d\n",
            obj->uid.uid,
            obj->uid.name,
            rico_obj_type_string[obj->type],
            obj->trans.x, obj->trans.y, obj->trans.z,
            obj->rot.x,   obj->rot.y,   obj->rot.z,
            obj->scale.x, obj->scale.y, obj->scale.z,
            obj->mesh,
            obj->material);

    }
    return buf;
}

int object_serialize_0(const void *handle, const struct rico_file *file)
{
    const struct rico_object *obj = handle;
    fwrite(&obj->type,     sizeof(obj->type),    1, file->fs);
    fwrite(&obj->trans,    sizeof(obj->trans),   1, file->fs);
    fwrite(&obj->rot,      sizeof(obj->rot),     1, file->fs);
    fwrite(&obj->scale,    sizeof(obj->scale),   1, file->fs);
    fwrite(&obj->mesh,     sizeof(obj->mesh),    1, file->fs);
    fwrite(&obj->material, sizeof(obj->material), 1, file->fs);
    return rico_serialize(&obj->bbox, file);
}

int object_deserialize_0(void *_handle, const struct rico_file *file)
{
    enum rico_error err;
    u32 mesh, material;

    struct rico_object *obj = _handle;
    fread(&obj->type,    sizeof(obj->type),     1, file->fs);
    fread(&obj->trans,   sizeof(obj->trans),    1, file->fs);
    fread(&obj->rot,     sizeof(obj->rot),      1, file->fs);
    fread(&obj->scale,   sizeof(obj->scale),    1, file->fs);
    fread(&mesh,         sizeof(obj->mesh),     1, file->fs);
    fread(&material,     sizeof(obj->material), 1, file->fs);

    update_transform(obj);
    obj->mesh     = mesh_request(mesh);
    obj->material = material_request(material);

    err = rico_deserialize(&obj->bbox, file);
    if (err == ERR_SERIALIZE_DISABLED)
    {
        obj->bbox = *mesh_bbox(obj->mesh);
    }
    return err;
}

struct rico_pool *object_pool_get_unsafe()
{
    return objects;
}

void object_pool_set_unsafe(struct rico_pool *pool)
{
    RICO_ASSERT(!objects);
    objects = pool;
}