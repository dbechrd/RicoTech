#include "rico_cereal.h"
#include "rico_uid.h"
#include "const.h"

Serializer RicoSerializers[RICO_UID_COUNT] = { NULL };
Deserializer RicoDeserializers[RICO_UID_COUNT] = { NULL };

static int rico_serialize_internal(const void *handle, FILE *fs)
{
    uid_serialize(handle, fs);
    const enum rico_uid_type *type = handle;

    if (!RicoSerializers[*type])
        return RICO_ERROR(ERR_SERIALIZER_NULL);

    return RicoSerializers[*type](handle, fs);
}

static int rico_deserialize_internal(void *_handle, FILE *fs)
{
    uid_deserialize(_handle, fs);
    enum rico_uid_type *type = _handle;

    if (!RicoDeserializers[*type])
        return RICO_ERROR(ERR_DESERIALIZER_NULL);

    return RicoDeserializers[*type](_handle, fs);
}

Serializer rico_serialize = rico_serialize_internal;
Deserializer rico_deserialize = rico_deserialize_internal;
