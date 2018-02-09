global u32 next_uid = 1;

void hnd_init(struct hnd *hnd, enum rico_hnd_type type, const char *name)
{
    hnd->type = type;

    // HACK: Don't assign ID numbers to objects which are rapidly created and
    //       destroyed. Fix this by reusing font meshes and bounding boxes.
    //       Also, we could garbage collect UIDs when UID_MAX is hit?
    // switch (type)
    // {
    // case RICO_HND_TEXTURE:
    // case RICO_HND_MESH:
    //     _hnd->uid = 0;
    //     break;
    // default:
    //     _hnd->uid = next_uid++;
    // }

    hnd->uid = (hnd->chunk == chunk_transient) ? UID_NULL : next_uid++;
    strncpy(hnd->name, name, sizeof(hnd->name) - 1);
    u32 name_len = dlb_strlen(name);
    hnd->len = MIN(name_len, sizeof(hnd->name) - 1);

#if RICO_DEBUG_HND
    printf("[ hnd][init] uid=%d type=%s name=%s", hnd->uid,
           rico_hnd_type_string[hnd->type], hnd->name);

    if (name_len > sizeof(hnd->name) - 1)
        printf("...");
    printf("\n");

#endif
}

#if 0
//int hnd_serialize(const void *handle, const struct rico_file *file)
SERIAL(hnd_serialize)
{
    // TODO: Do a single write
    const struct hnd *hnd = handle;
    fwrite(&hnd->type, sizeof(hnd->type), 1, file->fs);
    fwrite(&hnd->uid,  sizeof(hnd->uid),  1, file->fs);
    fwrite(&hnd->len,  sizeof(hnd->len),  1, file->fs);
    fwrite(&hnd->name, hnd->len,          1, file->fs);
    return SUCCESS;
}

//int hnd_deserialize(void *handle, const struct rico_file *file)
DESERIAL(hnd_deserialize)
{
    // TODO: Do a single read
    fread(&handle->type, sizeof(handle->type), 1, file->fs);
    fread(&handle->uid,  sizeof(handle->uid),  1, file->fs);
    fread(&handle->len,  sizeof(handle->len),  1, file->fs);
    fread(&handle->name, handle->len,          1, file->fs);
    return SUCCESS;
}
#endif