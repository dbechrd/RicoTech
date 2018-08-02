static size_t ric_stream_write(struct ric_stream *stream, void *buf,
                               size_t size)
{
    size_t bytes = fwrite(buf, 1, size, stream->fp);
    RICO_ASSERT(bytes == size);
    for (u32 i = 0; i < bytes; ++i)
    {
        u8 byte = *((const u8 *)buf + i);
        printf(" %02x", byte);
    }
    printf("\n");
    return bytes;
}

static size_t ric_stream_read(struct ric_stream *stream, void *buf, size_t size)
{
    size_t bytes = fread(buf, 1, size, stream->fp);
    RICO_ASSERT(bytes == size);
    for (u32 i = 0; i < bytes; ++i)
    {
        u8 byte = *((const u8 *)buf + i);
        printf(" %02x", byte);
    }
    printf("\n");
    return bytes;
}

static void ric_stream_open(struct ric_stream *stream)
{
    RICO_ASSERT(stream->filename);
    RICO_ASSERT(stream->mode);
    const char *mode = (stream->mode == RIC_SAV_READ) ? "rb" : "wb";
    stream->fp = fopen(stream->filename, mode);

    if (stream->mode == RIC_SAV_WRITE)
    {
        stream->magic = RIC_SAV_MAGIC;
    }
    RIC_STREAM_RW(&stream->magic, sizeof(stream->magic));
    RICO_ASSERT(stream->magic == RIC_SAV_MAGIC);

    RIC_STREAM_RW(&stream->version, sizeof(stream->version));
    RIC_STREAM_RW(&stream->arena->size, sizeof(stream->arena->size));
    if (!stream->arena->buffer)
    {
        ric_arena_alloc(stream->arena, stream->arena->size);
    }
}

static void ric_stream_close(struct ric_stream *stream)
{
    fclose(stream->fp);
}