const char *rico_string_slot_string[] = {
    RICO_STRING_SLOTS(GEN_STRING)
};

internal inline struct rico_pool **string_pool_ptr(enum rico_persist persist)
{
    struct rico_chunk *chunk = chunk_active();
    RICO_ASSERT(chunk);
    RICO_ASSERT(chunk->pools[persist][POOL_STRINGS]);
    return &chunk->pools[persist][POOL_STRINGS];
}

internal inline struct rico_pool *string_pool(enum rico_persist persist)
{
    return *string_pool_ptr(persist);
}

internal inline struct rico_string *string_find(struct hnd handle)
{
    struct rico_string *string = pool_read(string_pool(handle.persist),
                                           handle.value);
    RICO_ASSERT(string);
    return string;
}

internal inline struct rico_string *string_find_slot(enum rico_persist persist,
                                                     u32 slot)
{
    struct rico_string *string = pool_read(string_pool(persist), slot);
    RICO_ASSERT(string);
    return string;
}

int string_init(enum rico_persist persist, const char *name,
                enum rico_string_slot slot, float x, float y, struct col4 color,
                u32 lifespan, struct hnd font, const char *text)
{
    //RICO_ASSERT(slot <= STR_SLOT_COUNT || lifespan > 0);

#if RICO_DEBUG_STRING
    printf("[strg][init] name=%s\n", name);
#endif

    enum rico_error err;

    // Generate font mesh and get texture handle
    struct hnd text_mesh;
    struct hnd text_tex;
    err = font_render(&text_mesh, &text_tex, font, 0, 0, color, text, name,
                      MESH_STRING_SCREEN);
    if (err) return err;

    struct hnd text_material;
    err = material_init(&text_material, persist, name, text_tex,
                        RICO_DEFAULT_TEXTURE_SPEC, 0.5f);
    if (err) return err;

    // Generate dynamic string object or update existing fixed string
    struct rico_string *str;
    if (slot == STR_SLOT_DYNAMIC)
    {
        struct hnd handle;
        err = pool_handle_alloc(string_pool_ptr(persist), &handle);
        if (err) return err;
        str = string_find(handle);
    }
    else
    {
        // TODO: How to make this more logical? Should STR_SLOT_* be handles?
        struct hnd handle;
        handle.persist = persist;
        handle.value = slot;
        str = string_find(handle);
    }

    // TODO: Reuse mesh and material if they are the same
    // Reuse existing fixed string objects
    if (str->uid.uid != UID_NULL)
    {
        object_mesh_set(str->object, text_mesh, NULL);
        object_material_set(str->object, text_material);
    }
    else
    {
        uid_init(&str->uid, RICO_UID_STRING, name, false);

        // Create string object
        err = object_create(&str->object, persist, name, OBJ_STRING_SCREEN,
                            text_mesh, text_material, NULL, str->uid.serialize);
        if (err) return err;
    }

    str->lifespan = lifespan;
    object_trans_set(str->object,
                     &(struct vec3) { SCREEN_X(x), SCREEN_Y(y), -1.0f });

    return err;
}

int string_free(struct hnd handle)
{
    // // Preserve fixed string slots
    // if (handle < STR_SLOT_COUNT)
    //     return SUCCESS;

    enum rico_error err;
    struct rico_string *str = string_find(handle);

#if RICO_DEBUG_STRING
    printf("[strg][free] uid=%d name=%s\n", str->uid.uid, str->uid.name);
#endif

    if (handle.value < STR_SLOT_COUNT && str->uid.uid == UID_NULL)
    {
#if RICO_DEBUG_WARN
        printf("[strg][WARN] uid=%d handle=%d Static string already freed\n",
               str->uid.uid, handle.value);
#endif
        return SUCCESS;
    }

    object_free(str->object);
    str->object = HANDLE_NULL;

    str->uid.uid = UID_NULL;
    err = pool_handle_free(string_pool(handle.persist), handle);
    return err;
}

int string_update(r64 dt)
{
    enum rico_error err;
    u32 delta_ms = (u32)(dt * 1000);

    struct rico_string *str;

    for (u32 persist = 0; persist < PERSIST_COUNT; ++persist)
    {
        for (u32 i = 0; i < string_pool(persist)->active; ++i)
        {
            str = string_find(string_pool(persist)->handles[i]);
            if (str->uid.uid == UID_NULL || str->lifespan == 0)
            {
                continue;
            }

            // Free strings with expired lifespans
            if (str->lifespan <= delta_ms)
            {
                err = string_free(string_pool(persist)->handles[i]);
                if (err) return err;
            }
            else
            {
                str->lifespan -= delta_ms;
            }
        }
    }

    return SUCCESS;
}