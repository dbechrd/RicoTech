const char *rico_string_slot_string[] = {
    RICO_STRING_SLOTS(GEN_STRING)
};

int string_init(const char *name, enum rico_string_slot slot, float x, float y,
                struct col4 color, u32 lifespan, struct rico_font *font,
                const char *text)
{
    enum rico_error err;

#if RICO_DEBUG_STRING
    printf("[strg][init] name=%s\n", name);
#endif

    // Generate dynamic string object or update existing fixed string
    struct rico_string *str;
    if (slot != STR_SLOT_DYNAMIC)
    {
        // Look for previous slot string and delete it
        const char *slot_name = rico_string_slot_string[slot];
        str = (struct rico_string *)hashtable_search(&global_strings, slot_name,
                                                     strlen(slot_name));
        if (str)
        {
            hashtable_delete(&global_strings, slot_name, strlen(slot_name));
            string_free(str);
            str = NULL;
        }
    }

    // Allocate new string
    err = chunk_alloc(chunk_transient, RICO_HND_STRING, (struct hnd **)&str);
    if (err) return err;

    // TODO: Reuse mesh and material if they are the same
    // Generate font mesh and get texture handle
    struct rico_texture *text_mesh;
    struct rico_texture *text_tex;
    err = font_render(&text_mesh, &text_tex, font, 0, 0, color, text, name,
                      MESH_STRING_SCREEN);
    if (err) return err;

    struct rico_material *text_material;
    err = material_init(&text_material, name, text_tex,
                        RICO_DEFAULT_TEXTURE_SPEC, 0.5f);
    if (err) return err;

    // Store in global hash table
    err = hashtable_insert(&global_strings, str->hnd.name,
                           str->hnd.len, str);

    uid_init(&str->uid, RICO_UID_STRING, name, false);

    // Create string object
    err = object_init(&str->object, name, OBJ_STRING_SCREEN, text_mesh,
                      text_material, NULL);
    if (err) return err;

    str->lifespan = lifespan;
    object_trans_set(str->object,
                     &(struct vec3) { SCREEN_X(x), SCREEN_Y(y), -1.0f });

    return err;
}

int string_free(struct rico_string *str)
{
    // // Preserve fixed string slots
    // if (handle < STR_SLOT_COUNT)
    //     return SUCCESS;

#if RICO_DEBUG_STRING
    printf("[strg][free] uid=%d name=%s\n", str->hnd.uid, str->hnd.name);
#endif

    if (handle.value < STR_SLOT_COUNT && str->hnd.uid == UID_NULL)
    {
#if RICO_DEBUG_WARN
        printf("[strg][WARN] uid=%d handle=%d Static string already freed\n",
               str->hnd.uid, handle.value);
#endif
        return SUCCESS;
    }

    object_free(str->object);
    str->hnd.uid = UID_NULL;
    struct rico_pool *pool = chunk_pool(chunk_transient, RICO_HND_STRING);
    pool_handle_free(pool, &str->hnd);
    return err;
}

// TODO: Lifespan objects shouldn't be string-specific; refactor this logic out
//       into something more relevant, e.g. an object delete queue, sorted by
//       time to delete in ms, soonest first.
int string_update(r64 dt)
{
    enum rico_error err;
    u32 delta_ms = (u32)(dt * 1000);

    struct rico_pool *pool = chunk_pool(chunk_transient, RICO_HND_STRING);
    struct rico_string *str;
    for (u32 i = 0; i < pool->active; ++i)
    {
        str = (struct rico_string *)pool->handles[i];
        if (str->hnd.uid == UID_NULL || str->lifespan == 0)
            continue;

        // Free strings with expired lifespans
        if (str->lifespan <= delta_ms)
        {
            err = string_free(str);
            if (err) return err;
        }
        else
        {
            str->lifespan -= delta_ms;
        }
    }

    return SUCCESS;
}
