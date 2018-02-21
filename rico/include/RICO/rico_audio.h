#ifndef RICO_AUDIO_H
#define RICO_AUDIO_H

#include "AL/al.h"
#include "AL/alc.h"

#define RICO_AUDIO_STATE(f) \
    f(RICO_AUDIO_UNKNOWN)   \
    f(RICO_AUDIO_STOPPED)   \
    f(RICO_AUDIO_PLAYING)   \
    f(RICO_AUDIO_PAUSED)

enum rico_audio_state
{
    RICO_AUDIO_STATE(GEN_LIST)
    RICO_AUDIO_COUNT
};
const char *rico_audio_state_string[RICO_AUDIO_COUNT];

struct rico_audio_source
{
    ALuint al_source_id;
    float pitch;
    float gain;
    bool loop;
};

struct rico_audio_buffer
{
    ALuint al_buffer_id;
};

void RICO_audio_volume_set(float volume);
void RICO_audio_mute();
void RICO_audio_unmute();

void RICO_audio_source_init(struct rico_audio_source *source);
void RICO_audio_source_free(struct rico_audio_source *source);
void RICO_audio_source_buffer(struct rico_audio_source *source,
                              struct rico_audio_buffer *buffer);
void RICO_audio_source_play(struct rico_audio_source *source);
void RICO_audio_source_play_loop(struct rico_audio_source *source);
void RICO_audio_source_pause(struct rico_audio_source *source);
void RICO_audio_source_resume(struct rico_audio_source *source);
void RICO_audio_source_stop(struct rico_audio_source *source);
enum rico_audio_state RICO_audio_source_state(struct rico_audio_source *source);

void RICO_audio_buffer_load_file(struct rico_audio_buffer *buffer,
                                 const char *filename);
void RICO_audio_buffer_load(struct rico_audio_buffer *buffer, u32 len,
                            char *data);
void RICO_audio_buffer_free(struct rico_audio_buffer *buffer);

#endif