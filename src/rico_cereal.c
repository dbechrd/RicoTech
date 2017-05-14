//#include "rico_cereal.h"
//#include "rico_uid.h"
//#include "const.h"

struct rico_cereal rico_cereals[RICO_UID_COUNT] = { 0 };

//int rico_serialize(const void *handle, const struct rico_file *file)
SERIAL(rico_serialize)
{
    enum rico_error err;

    err = uid_serialize(handle, file);
    if (err) return err;

    const enum rico_uid_type *type = handle;

    if (!rico_cereals[*type].save[file->cereal_index])
        return RICO_ERROR(ERR_SERIALIZER_NULL, "Serializer null for type %s",
                          rico_uid_type_string[*type]);

    return rico_cereals[*type].save[file->cereal_index](handle, file);
}

//int rico_deserialize(void *_handle, const struct rico_file *file)
DESERIAL(rico_deserialize)
{
    enum rico_error err;

    err = uid_deserialize(_handle, file);
    if (err) return err;

    enum rico_uid_type *type = *_handle;

    if (!rico_cereals[*type].load[file->cereal_index])
        return RICO_ERROR(ERR_DESERIALIZER_NULL,
                          "Deserializer null for type %s",
                          rico_uid_type_string[*type]);

    err = rico_cereals[*type].load[file->cereal_index](_handle, file);
    return err;
}
