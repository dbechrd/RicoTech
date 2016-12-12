#include "rico_cereal.h"
#include "rico_uid.h"
#include "const.h"

struct rico_cereal RicoCereal[RICO_UID_COUNT] = { 0 };

int rico_serialize(const void *handle, const struct rico_file *file)
{
    RICO_ASSERT(file->version < RICO_FILE_VERSION_COUNT);

    // Don't serialize debug string objects to save file
    // TODO: Add "persistent" flag to objects
    // const struct rico_uid *uid = handle;
    // if (uid->type == RICO_UID_OBJECT) {
    //     const struct rico_obj *obj = handle;
    //     if (obj->type == OBJ_STRING_SCREEN)
    //         return SUCCESS;
    // }

    uid_serialize(handle, file);
    const enum rico_uid_type *type = handle;

    if (!RicoCereal[*type].save[file->version])
        return RICO_ERROR(ERR_SERIALIZER_NULL);

    return RicoCereal[*type].save[file->version](handle, file);
}

int rico_deserialize(void *_handle, const struct rico_file *file)
{
    RICO_ASSERT(file->version < RICO_FILE_VERSION_COUNT);

    uid_deserialize(_handle, file);
    enum rico_uid_type *type = _handle;

    if (!RicoCereal[*type].load[file->version])
        return RICO_ERROR(ERR_DESERIALIZER_NULL);

    return RicoCereal[*type].load[file->version](_handle, file);
}
