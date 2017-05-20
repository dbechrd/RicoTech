global u32 next_uid = 1;

const char *rico_uid_type_string[] = {
    RICO_UID_TYPES(GEN_STRING)
};

void uid_init(struct rico_uid *_uid, enum rico_uid_type type, const char *name,
              bool serialize)
{
    _uid->type = type;

    // HACK: Don't assign ID numbers to objects which are rapidly created and
    //       destroyed. Fix this by reusing font meshes and bounding boxes.
    // switch (type)
    // {
    // case RICO_UID_TEXTURE:
    // case RICO_UID_MESH:
    //     _uid->uid = 0;
    //     break;
    // default:
    //     _uid->uid = next_uid++;
    // }

    _uid->uid = next_uid++;

    strncpy(_uid->name, name, sizeof(_uid->name));
    _uid->serialize = serialize;

#if RICO_DEBUG_UID
    printf("[ uid][init] uid=%d type=%s name=%s", _uid->uid,
           rico_uid_type_string[_uid->type], _uid->name);

    if (strlen(name) > sizeof(_uid->name))
        printf("...");
    printf("\n");

#endif
}

//int uid_serialize(const void *handle, const struct rico_file *file)
SERIAL(uid_serialize)
{
    const struct rico_uid *uid = handle;
    if (!uid->serialize) {
        enum rico_uid_type type = RICO_UID_NULL;
        fwrite(&type, sizeof(type), 1, file->fs);
        return ERR_SERIALIZE_DISABLED;
    }

    // TODO: Do a single write
    u32 name_length = strlen(uid->name);
    fwrite(&uid->type,   sizeof(uid->type),   1, file->fs);
    fwrite(&uid->uid,    sizeof(uid->uid),    1, file->fs);
    fwrite(&name_length, sizeof(name_length), 1, file->fs);
    fwrite(&uid->name,   name_length,         1, file->fs);
    return SUCCESS;
}

//int uid_deserialize(void *_handle, const struct rico_file *file)
DESERIAL(uid_deserialize)
{
    struct rico_uid *uid = *_handle;
    fread(&uid->type, sizeof(uid->type), 1, file->fs);
    if (uid->type == RICO_UID_NULL)
    {
        return ERR_SERIALIZE_DISABLED;
    }

    // TODO: Do a single read
    u32 name_length;
    fread(&uid->uid,    sizeof(uid->uid),    1, file->fs);
    fread(&name_length, sizeof(name_length), 1, file->fs);
    fread(&uid->name,   name_length,         1, file->fs);
    uid->serialize = true;
    return SUCCESS;
}
