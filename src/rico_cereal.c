#if 0
struct rico_cereal rico_cereals[RICO_HND_CEREAL_COUNT] = { 0 };

//int rico_serialize(const hnd *handle, const struct rico_file *file)
SERIAL(rico_serialize)
{
    enum rico_error err;

    RICO_ASSERT(handle->uid);

    err = hnd_serialize(handle, file);
    if (err) return err;

    if (!rico_cereals[handle->type].save[file->cereal_index])
        return RICO_ERROR(ERR_SERIALIZER_NULL, "Serializer null for type %s",
                          rico_hnd_type_string[handle->type]);

    return rico_cereals[handle->type].save[file->cereal_index](handle, file);
}

//int rico_deserialize(struct hnd *handle, const struct rico_file *file)
DESERIAL(rico_deserialize)
{
    enum rico_error err;

    err = hnd_deserialize(handle, file);
    if (err) return err;

    if (!rico_cereals[handle->type].load[file->cereal_index])
        return RICO_ERROR(ERR_DESERIALIZER_NULL,
                          "Deserializer null for type %s",
                          rico_hnd_type_string[handle->type]);

    err = rico_cereals[handle->type].load[file->cereal_index](handle, file);
    return err;
}
#endif