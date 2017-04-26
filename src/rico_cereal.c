#include "rico_cereal.h"
#include "rico_uid.h"
#include "const.h"

struct rico_cereal rico_cereals[RICO_UID_COUNT] = { 0 };

//int rico_serialize(const void *handle, const struct rico_file *file)
SERIAL(rico_serialize)
{
    RICO_ASSERT(file->version < RICO_FILE_VERSION_COUNT);
    enum rico_error err;

    err = uid_serialize(handle, file);
    if (err) return err;

    const enum rico_uid_type *type = handle;

    if (!rico_cereals[*type].save[file->version])
        return RICO_ERROR(ERR_SERIALIZER_NULL);

    return rico_cereals[*type].save[file->version](handle, file);
}

//int rico_deserialize(void *_handle, const struct rico_file *file)
DESERIAL(rico_deserialize)
{
    RICO_ASSERT(file->version < RICO_FILE_VERSION_COUNT);
    enum rico_error err;

    err = uid_deserialize(_handle, file);
    if (err) return err;

    enum rico_uid_type *type = _handle;

    if (!rico_cereals[*type].load[file->version])
        return RICO_ERROR(ERR_DESERIALIZER_NULL);

    err = rico_cereals[*type].load[file->version](_handle, file);
    return err;
}
