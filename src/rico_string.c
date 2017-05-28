const u32 RICO_STRING_SIZE = sizeof(struct rico_string);

const char *rico_string_slot_string[] = {
    RICO_STRING_SLOTS(GEN_STRING)
};

internal inline struct rico_pool **string_pool_ptr()
{
    struct rico_chunk *chunk = chunk_active();
    RICO_ASSERT(chunk);
    RICO_ASSERT(chunk->strings);
    return &chunk->strings;
}

internal inline struct rico_pool *string_pool()
{
    return *string_pool_ptr();
}

internal inline struct rico_string *string_find(u32 handle)
{
    struct rico_string *string = pool_read(string_pool(), handle);
    RICO_ASSERT(string);
    return string;
}

int string_init(const char *name, enum rico_string_slot slot, float x, float y,
                struct col4 color, u32 lifespan, u32 font, const char *text)
{
    //RICO_ASSERT(slot <= STR_SLOT_COUNT || lifespan > 0);

#if RICO_DEBUG_STRING
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
    err = material_init(name, text_tex, RICO_DEFAULT_TEXTURE_SPEC, 0.5f,
                        &text_material);
    if (err) return err;

    // Generate dynamic string object or update existing fixed string
    struct rico_string *str;
    if (slot == STR_SLOT_DYNAMIC)
    {
        u32 handle;
        err = pool_handle_alloc(string_pool_ptr(), &handle);
        if (err) return err;
        str = string_find(handle);
    }
    else
    {
        str = string_find(slot);
    }

    // TODO: Reuse mesh and material if they are the same
    // Reuse existing fixed string objects
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
    // // Preserve fixed string slots
    // if (handle < STR_SLOT_COUNT)
    //     return SUCCESS;

    enum rico_error err;
    struct rico_string *str = string_find(handle);

#if RICO_DEBUG_STRING
    printf("[strg][free] uid=%d name=%s\n", str->uid.uid, str->uid.name);
#endif

    if (handle < STR_SLOT_COUNT && str->uid.uid == UID_NULL)
    {
#if RICO_DEBUG_WARN
        printf("[strg][WARN] uid=%d handle=%d Static string already freed\n",
               str->uid.uid, handle);
#endif
        return SUCCESS;
    }

    object_free(str->obj_handle);
    str->obj_handle = 0;

    str->uid.uid = UID_NULL;
    err = pool_handle_free(string_pool(), handle);
    return err;
}

int string_update(r64 dt)
{
    enum rico_error err;
    u32 delta_ms = (u32)(dt * 1000);

    u32 *handles = string_pool()->handles;
    struct rico_string *str;
    for (u32 i = 0; i < string_pool()->active; ++i)
    {
        str = string_find(handles[i]);
        if (str->uid.uid == UID_NULL || str->lifespan == 0)
        {
            continue;
        }

        // Free strings with expired lifespans
        if (str->lifespan <= delta_ms)
        {
            err = string_free(handles[i]);
            if (err) return err;
        }
        else
        {
            str->lifespan -= delta_ms;
        }
    }

    return SUCCESS;
}