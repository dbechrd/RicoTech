global u32 next_uid = 1;

const char *rico_hnd_type_string[] = {
    RICO_HND_TYPES(GEN_STRING)
};

void hnd_init(struct hnd *_hnd, enum rico_hnd_type type, const char *name,
              bool serialize)
{
    _hnd->type = type;

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

    _hnd->uid = next_uid++;

    strncpy(_hnd->name, name, sizeof(_hnd->name) - 1);
    _hnd->serialize = serialize;

#if RICO_DEBUG_HND
    printf("[ hnd][init] uid=%d type=%s name=%s", _hnd->uid,
           rico_hnd_type_string[_hnd->type], _hnd->name);

    if (strlen(name) > sizeof(_hnd->name) - 1)
        printf("...");
    printf("\n");

#endif
}

//int hnd_serialize(const void *handle, const struct rico_file *file)
SERIAL(hnd_serialize)
{
    const struct hnd *hnd = handle;
    if (!hnd->serialize) {
        enum rico_hnd_type type = RICO_HND_NULL;
        fwrite(&type, sizeof(type), 1, file->fs);
        return ERR_SERIALIZE_DISABLED;
    }

    // TODO: Do a single write
    u32 name_length = strlen(hnd->name);
    fwrite(&hnd->type,   sizeof(hnd->type),   1, file->fs);
    fwrite(&hnd->uid,    sizeof(hnd->uid),    1, file->fs);
    fwrite(&name_length, sizeof(name_length), 1, file->fs);
    fwrite(&hnd->name,   name_length,         1, file->fs);
    return SUCCESS;
}

//int hnd_deserialize(void *_handle, const struct rico_file *file)
DESERIAL(hnd_deserialize)
{
    struct hnd *hnd = *_handle;
    fread(&hnd->type, sizeof(hnd->type), 1, file->fs);
    if (hnd->type == RICO_HND_NULL)
    {
        return ERR_SERIALIZE_DISABLED;
    }

    // TODO: Do a single read
    u32 name_length;
    fread(&hnd->uid,    sizeof(hnd->uid),    1, file->fs);
    fread(&name_length, sizeof(name_length), 1, file->fs);
    fread(&hnd->name,   name_length,         1, file->fs);
    hnd->serialize = true;
    return SUCCESS;
}
