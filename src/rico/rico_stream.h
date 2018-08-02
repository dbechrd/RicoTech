#ifndef RICO_STREAM_H
#define RICO_STREAM_H

static size_t ric_stream_write(struct ric_stream *stream, void *buf,
                               size_t size);
static size_t ric_stream_read(struct ric_stream *stream, void *buf,
                              size_t size);
static void ric_stream_open(struct ric_stream *stream);
static void ric_stream_close(struct ric_stream *stream);

#endif