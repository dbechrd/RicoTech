struct rico_cereal rico_cereals[RICO_HND_COUNT] = { 0 };

//int rico_serialize(const void *handle, const struct rico_file *file)
SERIAL(rico_serialize)
{
    enum rico_error err;

    struct hnd *hnd = (struct hnd *)handle;
    RICO_ASSERT(hnd->uid);

    err = hnd_serialize(handle, file);
    if (err) return err;

    if (!rico_cereals[hnd->type].save[file->cereal_index])
        return RICO_ERROR(ERR_SERIALIZER_NULL, "Serializer null for type %s",
                          rico_hnd_type_string[hnd->type]);

    return rico_cereals[hnd->type].save[file->cereal_index](handle, file);
}

//int rico_deserialize(void *_handle, const struct rico_file *file)
DESERIAL(rico_deserialize)
{
    enum rico_error err;

    err = hnd_deserialize(_handle, file);
    if (err) return err;

    enum rico_hnd_type *type = *_handle;

    if (!rico_cereals[*type].load[file->cereal_index])
        return RICO_ERROR(ERR_DESERIALIZER_NULL,
                          "Deserializer null for type %s",
                          rico_hnd_type_string[*type]);

    err = rico_cereals[*type].load[file->cereal_index](_handle, file);
    return err;
}
