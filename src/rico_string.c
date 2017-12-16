const char *rico_string_slot_string[] = {
    RICO_STRING_SLOTS(GEN_STRING)
};

int string_init(struct rico_string *str, const char *name,
                enum rico_string_slot slot, float x, float y, struct col4 color,
                u32 lifespan, struct rico_font *font, const char *text)
{
    enum rico_error err;

#if RICO_DEBUG_STRING
    printf("[strg][init] name=%s\n", name);
#endif

    const char *slot_name = rico_string_slot_string[slot];

    if (slot != STR_SLOT_DYNAMIC)
    {
        // Look for previous slot string and delete it
        struct rico_string *old_str =
            hashtable_search_str(&global_string_slots, slot_name);
        if (old_str)
        {
            RICO_ASSERT(old_str->hnd.uid);
            string_free(old_str);
        }
    }

    // TODO: Reuse mesh and material if they are the same
    // Generate font mesh and get texture handle
    struct pool_id font_mesh_id;
    struct pool_id font_tex_id;
    struct pool_id font_id;
    struct rico_font *font;
    err = chunk_alloc(&font, str->hnd.chunk, &font_id, RICO_HND_FONT);
    if (err) return err;
    err = font_render(&font_mesh_id, &font_tex_id, font, 0, 0, color,
                      text, name, MESH_STRING_SCREEN);
    if (err) return err;

    struct pool_id font_material_id;
    struct rico_material *material;
    err = pool_add(&material, str->hnd.chunk->pools[RICO_HND_MATERIAL],
                   &font_material_id);
    if (err) return err;
    err = material_init(material, name, font_tex_id, RICO_DEFAULT_TEXTURE_SPEC,
                        0.5f);
    if (err) return err;

    // Init string
    hnd_init(&str->hnd, RICO_HND_STRING, name);
    str->slot = slot;

    struct rico_object *str_obj;
    err = pool_add(&str_obj, str->hnd.chunk->pools[RICO_HND_OBJECT], &str->object_id);
    if (err) return err;
    err = object_init(str_obj, name, OBJ_STRING_SCREEN, font_mesh_id,
                      font_material_id, NULL);
    if (err) return err;

    str->lifespan = lifespan;
    object_trans_set(str_obj, &(struct vec3) { SCREEN_X(x), SCREEN_Y(y), -1.0f });

    // Store in global hash table
    err = hashtable_insert_hnd(&global_strings, &str->hnd, str);

    // Store in slot table if not dynamic
    if (slot != STR_SLOT_DYNAMIC)
    {
        hashtable_insert_str(&global_string_slots, slot_name, str);
    }

    return err;
}

int string_free(struct rico_string *str)
{
#if RICO_DEBUG_STRING
    printf("[strg][free] uid=%d name=%s\n", str->hnd.uid, str->hnd.name);
#endif

    if (str->slot != STR_SLOT_DYNAMIC)
    {
        // Look for slot string and delete it
        hashtable_delete_str(&global_string_slots,
                             rico_string_slot_string[str->slot]);
    }

    chunk_free(str->hnd.chunk, str->object_id);
    hashtable_delete_hnd(&global_strings, &str->hnd);
    return pool_remove(str->hnd.pool, str->hnd.id);
}

// TODO: Lifespan objects shouldn't be string-specific; refactor this logic out
//       into something more relevant, e.g. an object delete queue, sorted by
//       time to delete in ms, soonest first.
int string_update(r64 dt)
{
    enum rico_error err;
    u32 delta_ms = (u32)(dt * 1000);

    struct rico_pool *pool = chunk_transient->pools[RICO_HND_STRING];
    struct rico_string *str = pool_last(pool);
    while (str)
    {
        if (str->lifespan > 0)
        {
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

        str = pool_prev(pool, str);
    }

    return SUCCESS;
}
