struct rico_physics *make_physics(struct vec3 size)
{
    struct rico_physics *phys = calloc(1, sizeof(struct rico_physics));

    phys->size = size;

    return phys;
}

void free_physics(struct rico_physics *phys)
{
    free(phys);
    phys = NULL;
}

void update_physics(struct rico_physics *phys, int count)
{
    for (int i = 0; i < count; i++)
    {
        phys->vel.x += phys->acc.x;
        phys->vel.y += phys->acc.y;
        phys->vel.z += phys->acc.z;

        phys->pos.x += phys->vel.x;
        phys->pos.y += phys->vel.y;
        phys->pos.z += phys->vel.z;
    }
}