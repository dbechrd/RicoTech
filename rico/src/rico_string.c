const char *rico_string_slot_string[] = {
    RICO_STRING_SLOTS(GEN_STRING)
};

void string_delete(struct pack *pack, struct rico_string *str)
{
#if RICO_DEBUG_STRING
    printf("[strg][free] uid=%d\n", str->id);
#endif

    if (str->slot != STR_SLOT_DYNAMIC)
    {
        // Look for slot string and delete it
        hashtable_delete(&global_string_slots, &str->slot, sizeof(str->slot));
    }

    pack_delete(str->object_id);
}

bool string_free_slot(enum rico_string_slot slot)
{
    if (slot != STR_SLOT_DYNAMIC)
    {
        // Look for previous slot string and delete it
        u32 *id = hashtable_search(&global_string_slots, &slot, sizeof(slot));
        if (id)
        {
            pack_delete(*id);
            return true;
        }
    }

    return false;
}

// TODO: Lifespan objects shouldn't be string-specific; refactor this logic out
//       into something more relevant, e.g. an object delete queue, sorted by
//       time to delete in ms, soonest first.
void string_update()
{
    struct pack *pack = RICO_packs[PACK_TRANSIENT];
    struct rico_string *str = 0;
    u32 index = 0;
    while(index < pack->blobs_used)
    {
        if (pack->index[index].type != RICO_HND_STRING)
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
