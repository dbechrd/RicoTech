#ifndef RICO_CHUNK
#define RICO_CHUNK

struct rico_chunk {
    char *data;
};

int chunk_save(const char *filename, const struct rico_chunk *data);
int chunk_load(const char *filename, struct rico_chunk *_data);

#endif //RICO_CHUNK