int alloc_material(struct rico_material **material, struct rico_pool *pool)
{
    enum rico_error err;

    struct hnd *hnd;
    err = pool_handle_alloc(pool, &hnd);
    if (err) return err;

    return (struct rico_material *)hnd;
}
