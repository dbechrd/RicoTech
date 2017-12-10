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

    // TODO: Reuse mesh and material if they are the same
    // Generate font mesh and get texture handle
    struct rico_mesh *mesh;
    struct rico_texture *tex;
    err = font_render(&mesh, &tex, font, 0, 0, color, text, name,
                      MESH_STRING_SCREEN);
    if (err) return err;

    struct rico_material *material;
    err = chunk_alloc(chunk_transient, RICO_HND_MATERIAL,
                      (struct hnd **)&material);
    if (err) return err;
    err = material_init(material, name, tex, RICO_DEFAULT_TEXTURE_SPEC, 0.5f);
    if (err) return err;

    hnd_init(&str->hnd, RICO_HND_STRING, name);

    // Create string object
    str->slot = slot;
    err = chunk_alloc(chunk_transient, RICO_HND_OBJECT,
                      (struct hnd **)&str->object);
    if (err) return err;
    err = object_init(str->object, name, OBJ_STRING_SCREEN, mesh, material,
                      NULL);
    if (err) return err;

    str->lifespan = lifespan;
    object_trans_set(str->object,
                     &(struct vec3) { SCREEN_X(x), SCREEN_Y(y), -1.0f });

    // Store in global hash table
    err = hashtable_insert_hnd(&global_strings, &str->hnd, str);

    // Store in slot table if not dynamic
    if (slot != STR_SLOT_DYNAMIC)
    {
        const char *slot_name = rico_string_slot_string[slot];

        // Look for previous slot string and delete it
        struct rico_string *old_str =
            hashtable_search_str(&global_string_slots, slot_name);
        if (old_str)
        {
            hashtable_delete_str(&global_string_slots, slot_name);
            string_free(old_str);
        }

        // Insert new string in string slot hash
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
        hashtable_delete_str(&global_strings,
                             rico_string_slot_string[str->slot]);
    }

    object_free(str->object);
    str->hnd.uid = UID_NULL;
    return chunk_free(chunk_transient, &str->hnd);
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
