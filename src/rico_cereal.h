#ifndef RICO_CEREAL_H
#define RICO_CEREAL_H

#if 0
#define SERIAL(name) int name(const struct hnd *handle, \
                              const struct rico_file *file)
#define DESERIAL(name) int name(struct hnd *handle, \
                                const struct rico_file *file)
typedef SERIAL(serializer);
typedef DESERIAL(deserializer);

struct rico_cereal
{
    serializer *save[RICO_FILE_VERSION_COUNT];
    deserializer *load[RICO_FILE_VERSION_COUNT];
};

extern struct rico_cereal rico_cereals[];

SERIAL(rico_serialize);
DESERIAL(rico_deserialize);
#endif

#endif