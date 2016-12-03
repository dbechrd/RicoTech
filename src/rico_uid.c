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
    printf("[UID %s %d][%s] Init", rico_uid_type_string[_uid->type],
           _uid->uid, _uid->name);
    if (strlen(name) > sizeof(_uid->name))
    {
        printf(" (Truncated)\n");
    }
    else
    {
        printf("\n");
    }
#endif
}

int uid_serialize(const void *handle, const struct rico_file *file)
{
    const struct rico_uid *uid = handle;
    uint32 name_length = strlen(uid->name);

    fwrite(&uid->type,    sizeof(uid->type),    1, file->fs);
    fwrite(&uid->uid,     sizeof(uid->uid),     1, file->fs);
    fwrite(&name_length,  sizeof(name_length),  1, file->fs);
    fwrite(&uid->name,    name_length,          1, file->fs);
    return SUCCESS;
}

int uid_deserialize(void *_handle, const struct rico_file *file)
{
    struct rico_uid *uid = _handle;
    uint32 name_length;

    fread(&uid->type,    sizeof(uid->type),    1, file->fs);
    fread(&uid->uid,     sizeof(uid->uid),     1, file->fs);
    fread(&name_length,  sizeof(name_length),  1, file->fs);
    fread(&uid->name,    name_length,          1, file->fs);
    return SUCCESS;
}
