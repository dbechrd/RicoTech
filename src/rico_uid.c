#include "rico_uid.h"
#include <string.h>
#include <stdio.h>

struct rico_uid UID_NULL = { RICO_UID_NULL, 0, "NULL", NULL, NULL };
static uint32 next_uid = 1;

const char *rico_uid_type_string[] = {
    RICO_UID_TYPES(GEN_STRING)
};

void uid_init(struct rico_uid *_uid, enum rico_uid_type type, const char *name)
{
    _uid->type = type;
    _uid->uid = next_uid++;
    strncpy(_uid->name, name, sizeof(_uid->name));

#ifdef RICO_DEBUG_UID
    printf("[UID %s %d][%s] Initialized\n", rico_uid_type_string[_uid->type],
           _uid->uid, _uid->name);
#endif
}

Serializer RicoSerializers[RICO_UID_COUNT] = { NULL };
Deserializer RicoDeserializers[RICO_UID_COUNT] = { NULL };

static int rico_serialize(const void *handle, FILE *fs)
{
    const enum rico_uid_type *type = handle;
    if (!RicoSerializers[*type])
        return RICO_ERROR(ERR_SERIALIZER_NULL);

    return RicoSerializers[*type](handle, fs);
}

static int rico_deserialize(void *_handle, FILE *fs)
{
    enum rico_uid_type *type = _handle;
    if (!RicoDeserializers[*type])
        return RICO_ERROR(ERR_DESERIALIZER_NULL);

    return RicoDeserializers[*type](_handle, fs);
}

Serializer rico_serializer = rico_serialize;
Deserializer rico_deserializer = rico_deserialize;