pkid global_string_slots[RIC_STRING_SLOT_COUNT + 64];

static void string_delete(struct ric_string *str)
{
#if RICO_DEBUG_STRING
    printf("[strg][free] uid=%d\n", str->id);
#endif

    global_string_slots[str->slot] = 0;

    pack_delete(str->mesh_id);
    // Note: Let font engine manage font texture
}
static void string_free_slot(enum ric_string_slot slot)
{
    if (global_string_slots[slot])
    {
        pack_delete(global_string_slots[slot]);
        global_string_slots[slot] = 0;
    }
}
// TODO: Lifespan objects shouldn't be string-specific; refactor this logic out
//       into something more relevant, e.g. an object delete queue, sorted by
//       time to delete in ms, soonest first.
static void string_update()
{
    struct pack *pack = global_packs[RIC_PACK_ID_TRANSIENT];
    struct ric_string *str = 0;
    u32 index = 0;
    while(index < pack->blobs_used)
    {
        if (pack->index[index].type != RIC_ASSET_STRING)
        {
            index++;
            continue;
        }

        str = pack_read(pack, index);
        if (str->lifespan > 0)
        {
            if (str->lifespan > (u32)SIM_MS)
            {
                str->lifespan -= (u32)SIM_MS;
            }
            else
            {
                pack_delete(str->uid.pkid);
                continue;
            }
        }

        index++;
    }
}

static void string_render(struct ric_string *str, GLint model_location)
{
    glUniformMatrix4fv(model_location, 1, GL_TRUE, MAT4_IDENT.a);

    texture_bind(str->texture_id, GL_TEXTURE0);
    glBindVertexArray(mesh_vao(str->mesh_id));
    mesh_render(str->mesh_id);
    glBindVertexArray(0);
    texture_unbind(str->texture_id, GL_TEXTURE0);
}

static void string_render_all(GLint model_location)
{
    for (u32 i = 0; i < ARRAY_COUNT(global_string_slots); ++i)
    {
        pkid id = global_string_slots[i];
        if (id)
        {
            struct ric_string *str = ric_pack_lookup(id);
            string_render(str, model_location);
        }
    }
}