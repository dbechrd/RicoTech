#include "rico_string.h"
#include "rico_uid.h"
#include "geom.h"
#include "rico_pool.h"
#include "rico_font.h"
#include "rico_object.h"
#include "camera.h"
#include "rico_material.h"

struct rico_string {
    struct rico_uid uid;
    u32 obj_handle;
    u32 lifespan;
};

const char *rico_string_slot_string[] = {
    RICO_STRING_SLOTS(GEN_STRING)
};

static struct rico_pool strings;

int rico_string_init(u32 pool_size)
{
    return pool_init("Strings", pool_size, sizeof(struct rico_string),
                     STR_SLOT_DYNAMIC, &strings);
}

int string_init(const char *name, enum rico_string_slot slot, u32 x, u32 y,
                struct col4 color, u32 lifespan, u32 font, const char *text)
{
    RICO_ASSERT(slot < STR_SLOT_COUNT || lifespan > 0);

#ifdef RICO_DEBUG_STRING
    printf("[strg][init] name=%s\n", name);
#endif

    enum rico_error err;

    // Generate font mesh and get texture handle
    u32 text_mesh;
    u32 text_tex;
    err = font_render(font, 0, 0, color, text, "debug_info_string",
                      MESH_STRING_SCREEN, &text_mesh, &text_tex);
    if (err) return err;

    u32 text_material;
    err = material_init(name, text_tex, RICO_TEXTURE_DEFAULT_SPEC, 0.5f,
                        &text_material);
    if (err) return err;

    // Generate dynamic string object or update existing static string
    struct rico_string *str;
    if (slot == STR_SLOT_DYNAMIC)
    {
        u32 handle;
        err = pool_handle_alloc(&strings, &handle);
        if (err) return err;
        str = pool_read(&strings, handle);
    }
    else
    {
        str = pool_read(&strings, slot);
    }

    // Reuse existing static string objects
    if (str->uid.uid != UID_NULL)
    {
        object_mesh_set(str->obj_handle, text_mesh, NULL);
        object_material_set(str->obj_handle, text_material);
    }
    else
    {
        uid_init(&str->uid, RICO_UID_STRING, name, false);

        // Create string object
        err = object_create(&str->obj_handle, name, OBJ_STRING_SCREEN,
                            text_mesh, text_material, NULL, str->uid.serialize);
        if (err) return err;
    }

    str->lifespan = lifespan;
    object_trans_set(str->obj_handle,
                     &(struct vec3) { SCREEN_X(x), SCREEN_Y(y), -1.0f });

    return err;
}

int string_free(u32 handle)
{
    // // Preserve static string slots
    // if (handle < STR_SLOT_COUNT)
    //     return SUCCESS;

    enum rico_error err;
    struct rico_string *str = pool_read(&strings, handle);

#ifdef RICO_DEBUG_STRING
    printf("[strg][free] name=%s\n", str->uid.name);
#endif

    if (handle < STR_SLOT_COUNT && str->uid.uid == UID_NULL)
    {
#ifdef RICO_DEBUG_WARN
        printf("[strg][WARN] handle=%d Static string already freed\n", handle);
#endif
        return SUCCESS;
    }

    object_free(str->obj_handle);
    str->obj_handle = 0;

    str->uid.uid = UID_NULL;
    err = pool_handle_free(&strings, handle);
    return err;
}

int string_update(r64 dt)
{
    enum rico_error err;

    u32 delta_ms = (u32)(dt * 1000);

    struct rico_string *str;
    for (u32 i = 0; i < strings.active; ++i)
    {
        str = pool_read(&strings, strings.handles[i]);
        if (str->uid.uid == UID_NULL || str->lifespan == 0)
        {
            continue;
        }

        // Free strings with expired lifespans
        if (str->lifespan <= delta_ms)
        {
            err = string_free(strings.handles[i]);
            if (err) return err;
        }
        else
        {
            str->lifespan -= delta_ms;
        }
    }

    return SUCCESS;
}