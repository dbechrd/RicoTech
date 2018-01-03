const char *rico_string_slot_string[] = {
    RICO_STRING_SLOTS(GEN_STRING)
};

int string_init(struct rico_string *str, const char *name,
                enum rico_string_slot slot, float x, float y, struct vec4 color,
                u32 lifespan, struct rico_font *font, const char *text)
{
    enum rico_error err = SUCCESS;

#if RICO_DEBUG_STRING
    printf("[strg][init] name=%s\n", name);
#endif

    // TODO: Reuse mesh and material if they are the same
    // Generate font mesh and get texture handle
    u32 font_mesh_id;
    u32 font_mat_id;
    font_render(&font_mesh_id, &font_mat_id, font, 0, 0, color, text, name);

    // Init string
    hnd_init(&str->hnd, RICO_HND_STRING, name);
    str->slot = slot;
    struct obj_property str_props[2] = { 0 };
    str_props[0].type = PROP_MESH_ID;
    str_props[0].mesh_id = font_mesh_id;
    str_props[1].type = PROP_MATERIAL_ID;
    str_props[1].material_id = font_mat_id;
    str->object_id = load_object(pack_transient, name, OBJ_STRING_SCREEN,
                                 array_count(str_props), str_props, NULL);
    struct rico_object *str_obj = pack_lookup(pack_transient, str->object_id);
    object_trans_set(str_obj, &VEC3(SCREEN_X(x), SCREEN_Y(y), -1.0f));
    
    str->lifespan = lifespan;

    // Store in global hash table
    //err = hashtable_insert_hnd(&global_strings, &str->hnd, &str->hnd.id,
    //                           sizeof(str->hnd.id));

    // Store in slot table if not dynamic
    if (slot != STR_SLOT_DYNAMIC)
    {
        const char *slot_name = rico_string_slot_string[slot];
        err = hashtable_insert_str(&global_string_slots, slot_name,
                                   &str->hnd.id, sizeof(str->hnd.id));
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

    //chunk_free(str->hnd.chunk, str->object_id);
    //hashtable_delete_hnd(&global_strings, &str->hnd);

    struct rico_object *obj = pack_lookup(pack_transient, str->object_id);
    //pack_delete(pack_default, obj->material_id, RICO_HND_MATERIAL);
    struct obj_property *mesh_prop = object_prop(obj, PROP_MESH_ID);
    RICO_ASSERT(mesh_prop);
    pack_delete(pack_transient, mesh_prop->mesh_id, RICO_HND_MESH);
    pack_delete(pack_transient, str->object_id, RICO_HND_OBJECT);

    return pool_remove(str->hnd.pool, str->hnd.id);
}

int string_free_slot(enum rico_string_slot slot)
{
    if (slot != STR_SLOT_DYNAMIC)
    {
        // Look for previous slot string and delete it
        struct pool_id *old_str_id =
            hashtable_search_str(&global_string_slots,
                                 rico_string_slot_string[slot]);
        if (old_str_id)
        {
            struct rico_string *old_str = chunk_read(chunk_transient,
                                                     *old_str_id);
            return string_free(old_str);
        }
    }

    return SUCCESS;
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
