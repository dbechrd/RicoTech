const char *rico_string_slot_string[] = {
    RICO_STRING_SLOTS(GEN_STRING)
};

void string_delete(struct pack *pack, struct rico_string *str)
{
#if RICO_DEBUG_STRING
    printf("[strg][free] uid=%d name=%s\n", str->hnd.uid, str->hnd.name);
#endif

    if (str->slot != STR_SLOT_DYNAMIC)
    {
        // Look for slot string and delete it
        hashtable_delete(&global_string_slots, &str->slot, sizeof(str->slot));
    }

    pack_delete(pack, str->object_id, RICO_HND_OBJECT);
}

bool string_free_slot(enum rico_string_slot slot)
{
    if (slot != STR_SLOT_DYNAMIC)
    {
        // Look for previous slot string and delete it
        u32 *id = hashtable_search(&global_string_slots, &slot, sizeof(slot));
        if (id)
        {
            pack_delete(pack_transient, *id, RICO_HND_STRING);
            return true;
        }
    }

    return false;
}

// TODO: Lifespan objects shouldn't be string-specific; refactor this logic out
//       into something more relevant, e.g. an object delete queue, sorted by
//       time to delete in ms, soonest first.
void string_update(r64 dt)
{
    u32 delta_ms = (u32)(dt * 1000);

    struct pack *pack = pack_transient;
    struct rico_string *str = 0;
    u32 index = 0;
    while(index < pack->blobs_used)
    {
        if (pack->index[index].type != RICO_HND_STRING)
        {
            index--;
            continue;
        }

        str = pack_read(pack, index);
        if (str->lifespan > 0)
        {
            if (str->lifespan > delta_ms)
            {
                str->lifespan -= delta_ms;
                index--;
            }
            else
            {
                pack_delete(pack_transient, str->id, RICO_HND_STRING);
            }
        }
    }
}
