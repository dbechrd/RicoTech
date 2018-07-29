static void pool_print_handles(struct rico_pool *pool);

inline void pool_fixup(struct rico_pool *pool, struct rico_chunk *chunk)
{
    // TODO: Could clean this up with PTR_ADD_BYTE macro
    pool->block_tags = (u32 *)
        ((u8 *)pool + POOL_BLOCK_TAGS_OFFSET());
    pool->tags = (struct pool_tag *)
        ((u8 *)pool + POOL_TAGS_OFFSET(pool->block_count));
    pool->blocks = (u8 *)pool + POOL_BLOCKS_OFFSET(pool->block_count);
    pool->end = pool->blocks + (pool->blocks_used * pool->block_size);
    
    // MUST set chunk when fixing up handles in use
    RICO_ASSERT(pool->blocks_used == 0 || chunk);

    struct hnd *hnd;
    for (u32 i = 0; i < pool->blocks_used; ++i)
    {
        hnd = (struct hnd *)(pool->blocks + (i * pool->block_size));
        hnd->chunk = chunk;
        hnd->pool = pool;
    }
}

// TODO: Allocate from heap pool, don't keep calling calloc
int pool_init(void *buf, const char *name, u32 block_count, u32 block_size)
{
    RICO_ASSERT(block_count > 0);
    RICO_ASSERT(block_size > 0);

    struct rico_pool *pool = buf;
    pool->uid = 0;
    strncpy(pool->name, name, sizeof(pool->name) - 1);
    pool->block_count = block_count;
    pool->block_size = block_size;
    pool->blocks_used = 0;
    pool->next_free = 0;

    // Pointer fix-up
    pool_fixup(pool, NULL);

    // Initialize block_tags and tags
    for (u32 i = 0; i < pool->block_count - 1; ++i)
    {
        pool->block_tags[i] = i;
        pool->tags[i].next_free = i + 1;
    }

#if RICO_DEBUG_POOL
    printf("[pool][init] %s\n", pool->name);
#endif

    return RIC_SUCCESS;
}

int pool_add(struct hnd **handle, struct rico_pool *pool)
{
    RICO_ASSERT(pool);

    enum ric_error err = RIC_SUCCESS;

    if (pool->blocks_used == pool->block_count)
    {
#if RICO_DEBUG_POOL
        printf("[pool][ ERR] %s out of memory. [%d] blocks used.\n",
               pool->name, pool->blocks_used);
        pool_print_handles(pool);
#endif

        return RICO_FATAL(RIC_ERR_POOL_OUT_OF_MEMORY, "%s pool is full",
                          pool->name);
    }

    struct hnd *hnd = (struct hnd *)pool->end;
    hnd->pool = pool;
    hnd->id.pool_uid = pool->uid;

    // Tag
    hnd->id.tag = pool->next_free;
    pool->next_free = pool->tags[hnd->id.tag].next_free;

    // Generation
    pool->tags[hnd->id.tag].generation++;
    hnd->id.generation = pool->tags[hnd->id.tag].generation;

    // Tags / Block tags
    pool->tags[hnd->id.tag].block = pool->blocks_used;
    pool->block_tags[pool->blocks_used] = hnd->id.tag;

    // Counts
    //pool->tags[hnd->id.tag].ref_count = 1;
    //hnd->ref_count = 1;
    pool->blocks_used++;
    pool->end = pool->blocks + (pool->blocks_used * pool->block_size);

    if (handle)
    {
        *handle = hnd;
    }

#if RICO_DEBUG_POOL
    printf("[pool][aloc] %s\n", pool->name);
    pool_print_handles(pool);
#endif

    return err;
}

int pool_remove(struct rico_pool *pool, struct pool_id id)
{
    RICO_ASSERT(id.pool_uid == pool->uid);

    if (pool->tags[id.tag].generation != id.generation)
    {
        return RICO_ERROR(RIC_ERR_POOL_BAD_FREE,
                          "Tag generation mismatch, tag %u already freed!",
                          id.tag);
    }

    u32 block_idx = pool->tags[id.tag].block;
    RICO_ASSERT(block_idx < pool->blocks_used);
    pool->blocks_used--;
    pool->end = pool->blocks + (pool->blocks_used * pool->block_size);

    if (block_idx < pool->blocks_used)
    {
        // Fill hole with last block to keep buffer contiguous
        memcpy(pool->blocks + (block_idx * pool->block_size),
               pool->blocks + (pool->blocks_used * pool->block_size),
               pool->block_size);

        // Update block index for the block that we moved
        u32 moved_tag = pool->block_tags[pool->blocks_used];
        pool->block_tags[block_idx] = moved_tag;
        pool->tags[moved_tag].block = block_idx;
    }

#if RICO_DEBUG
    // Zero memory block to force out-of-bounds errors while debugging
    memset(pool->blocks + (pool->blocks_used * pool->block_size),
           0, pool->block_size);
#endif

    pool->tags[id.tag].next_free = pool->next_free;
    pool->next_free = id.tag;

#if RICO_DEBUG_POOL
    printf("[pool][free] %s\n", pool->name);
    pool_print_handles(pool);
#endif

    return RIC_SUCCESS;
}

#if 0
inline void pool_request(struct rico_pool *pool, struct pool_id id)
{
    RICO_ASSERT(id.pool_uid == pool->uid);
    if (id.type)
    {
        pool->tags[id.tag].ref_count++;
    }
}
#endif

static void pool_print_handles(struct rico_pool *pool)
{
#if RICO_DEBUG_POOL
    printf("             [");
    if (pool->blocks_used == 0)
    {
        printf("EMPTY]\n");
        return;
    }

    // Print pool
    u32 vertex_count = 0;
    u32 free = pool->next_free;
    for (u32 i = 0; i < pool->block_count; i++)
    {
        if (i == free)
        {
            free = pool->tags[free].next_free;
            continue;
        }

        printf("%u", pool->tags[i].block);
        vertex_count++;
        if (vertex_count == pool->blocks_used) break;
        printf(" ");
    }
    printf("]\n");
    fflush(stdout);
#else
    UNUSED(pool);
#endif
}

inline void *pool_first(struct rico_pool *pool)
{
    if (pool->blocks_used == 0) return NULL;
    return pool->blocks;
}

inline void *pool_last(struct rico_pool *pool)
{
    if (pool->blocks_used == 0) return NULL;
    return pool->end - pool->block_size;
}

inline void *pool_next(struct rico_pool *pool, void *block)
{
    if (block == NULL) return pool_first(pool);
    block = (u8 *)block + pool->block_size;
    return (block == pool->end) ? NULL : block;
}

inline void *pool_prev(struct rico_pool *pool, void *block)
{
    if (block == NULL) return pool_last(pool);
    return (block == pool->blocks) ? NULL : (u8 *)block - pool->block_size;
}

inline void *pool_read(struct rico_pool *pool, struct pool_id id)
{
    RICO_ASSERT(id.pool_uid == pool->uid);
    return pool->blocks + (pool->tags[id.tag].block * pool->block_size);
}

inline struct pool_id pool_next_id(struct rico_pool *pool, struct pool_id id)
{
    RICO_ASSERT(id.pool_uid == pool->uid);
    RICO_ASSERT(pool->blocks_used);

    u32 block = pool->tags[id.tag].block;
    if (block == pool->blocks_used - 1)
        block = 0;
    else
        block++;

    struct pool_id new_id;
    new_id.type = id.type;
    new_id.tag = pool->block_tags[block];
    new_id.generation = pool->tags[new_id.tag].generation;
    return id;
}

inline struct pool_id pool_prev_id(struct rico_pool *pool, struct pool_id id)
{
    RICO_ASSERT(id.pool_uid == pool->uid);
    RICO_ASSERT(pool->blocks_used);

    u32 block = pool->tags[id.tag].block;
    if (block == 0)
        block = pool->blocks_used - 1;
    else
        block--;

    struct pool_id new_id;
    new_id.type = id.type;
    new_id.tag = pool->block_tags[block];
    new_id.generation = pool->tags[new_id.tag].generation;
    return id;
}

#if 0
struct hnd *pool_handle_next(struct rico_pool *pool, u32 idx)
{
    RICO_ASSERT(pool);

    // Nothing selected, return first item in pool
    if (!handle)
        return pool_handle_first(pool);

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i]->uid != handle->uid && i++ < pool->blocks_used) {}

    // Last item selected, return first item in pool
    if (i == pool->blocks_used)
        return pool_handle_first(pool);

    // Return next item
    return pool->handles[i];
}

struct hnd *pool_handle_prev(struct rico_pool *pool, u32 idx)
{
    RICO_ASSERT(pool);

    // Nothing selected, return last item in pool
    if (!handle)
        return pool_handle_last(pool);

    // First item selected, return last item in pool
    if (handle->uid == pool->handles[0]->uid)
        return pool->handles[pool->blocks_used - 1];

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i]->uid != handle->uid && i++ < pool->blocks_used) {}

    // Return previous item
    return pool->handles[i - 1];
}

struct hnd *pool_handle_first(struct rico_pool *pool)
{
    // Pool is empty, return null
    if (pool->blocks_used == 0)
        return NULL;

    return pool->handles[0];
}

struct hnd *pool_handle_last(struct rico_pool *pool)
{
    // Pool is empty, return null
    if (pool->blocks_used == 0)
        return NULL;

    return pool->handles[pool->blocks_used - 1];
}

struct hnd *pool_handle_next(struct rico_pool *pool, u32 idx)
{
    RICO_ASSERT(pool);

    // Nothing selected, return first item in pool
    if (!handle)
        return pool_handle_first(pool);

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i]->uid != handle->uid && i++ < pool->blocks_used) {}

    // Last item selected, return first item in pool
    if (i == pool->blocks_used)
        return pool_handle_first(pool);

    // Return next item
    return pool->handles[i];
}

struct hnd *pool_handle_prev(struct rico_pool *pool, u32 idx)
{
    RICO_ASSERT(pool);

    // Nothing selected, return last item in pool
    if (!handle)
        return pool_handle_last(pool);

    // First item selected, return last item in pool
    if (handle->uid == pool->handles[0]->uid)
        return pool->handles[pool->blocks_used - 1];

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i]->uid != handle->uid && i++ < pool->blocks_used) {}

    // Return previous item
    return pool->handles[i - 1];
}

//int pool_serialize_0(const void *handle, const struct rico_file *file)
SERIAL(pool_serialize_0)
{
    enum rico_error err;

    const struct rico_pool *pool = handle;
    fwrite(&pool->vertex_count,        sizeof(pool->vertex_count),        1, fp->fs);
    fwrite(&pool->min_size,         sizeof(pool->min_size),         1, fp->fs);
    fwrite(&pool->active,       sizeof(pool->active),       1, fp->fs);
    fwrite(pool->handles, sizeof(*pool->handles), pool->vertex_count, fp->fs);
    for (u32 i = 0; i < pool->active; ++i)
    {
        err = rico_serialize(pool_read(pool, pool->handles[i]), fp);
        if (err == RIC_ERR_SERIALIZE_DISABLED)
            continue;
        if (err)
            return err;
    }

    return RIC_SUCCESS;
}

//int pool_deserialize_0(void *_handle, const struct rico_file *file)
DESERIAL(pool_deserialize_0)
{
    enum rico_error err;

    struct rico_pool *pool = *_handle;
    fread(&pool->vertex_count,        sizeof(pool->vertex_count),        1, fp->fs);
    fread(&pool->min_size,         sizeof(pool->min_size),         1, fp->fs);
    fread(&pool->active,       sizeof(pool->active),       1, fp->fs);

    if(pool->vertex_count > 0)
    {
        pool->handles = calloc(pool->vertex_count, sizeof(u32));
        if (!pool->handles)
            return RICO_ERROR(RIC_ERR_BAD_ALLOC,
                              "Failed to alloc handles for pool %s",
                              pool->name);

        pool->data = calloc(pool->vertex_count, pool->min_size);
        if (!pool->data)
            return RICO_ERROR(RIC_ERR_BAD_ALLOC, "Failed to alloc data for pool %s",
                              pool->name);

        fread(pool->handles, sizeof(*pool->handles), pool->vertex_count, fp->fs);
        for (u32 i = 0; i < pool->active; ++i)
        {
            err = rico_deserialize(pool_read(pool, pool->handles[i]), fp);
            if (err == RIC_ERR_SERIALIZE_DISABLED)
                continue;
            if (err)
                return err;
        }

        if (pool->active > 0)
        {
            struct rico_uid *uid;
            struct handle handle;
            for (u32 i = 0; i < pool->active; ++i)
            {
                uid = pool_read(pool, pool->handles[i]);
                if (uid->uid == UID_NULL)
                {
                    pool->active--;
                    if (pool->active > 0)
                    {
                        handle = pool->handles[i];
                        pool->handles[i] = pool->handles[pool->active];
                        pool->handles[pool->active] = handle;
                    }
                }
            }
        }
    }

#if RICO_DEBUG_POOL
    printf("[pool][load] %s file %s\n",
           pool->name, fp->filename);
    pool_print_handles(pool);
#endif

    return RIC_SUCCESS;
}
#endif