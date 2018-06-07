static struct rico_physics *make_physics(struct vec3 min_size)
{
    struct rico_physics *phys = calloc(1, sizeof(struct rico_physics));
    phys->min_size = min_size;

    return phys;
}
static void free_physics(struct rico_physics *phys)
{
    free(phys);
    phys = NULL;
}
static void update_physics(struct rico_physics *phys, int bucket_count)
{
    for (int i = 0; i < bucket_count; i++)
    {
        phys->vel.x += phys->acc.x;
        phys->vel.y += phys->acc.y;
        phys->vel.z += phys->acc.z;

        phys->pos.x += phys->vel.x;
        phys->pos.y += phys->vel.y;
        phys->pos.z += phys->vel.z;
    }
}