#ifndef RICO_AUDIO_H
#define RICO_AUDIO_H

extern float RICO_audio_volume();
extern void RICO_audio_volume_set(float volume);
extern bool RICO_audio_muted();
extern void RICO_audio_mute();
extern void RICO_audio_unmute();
extern void RICO_audio_toggle();
extern void RICO_audio_source_init(struct RICO_audio_source *source);
extern void RICO_audio_source_free(struct RICO_audio_source *source);
extern void RICO_audio_source_buffer(struct RICO_audio_source *source,
                                     struct RICO_audio_buffer *buffer);
extern void RICO_audio_source_play(struct RICO_audio_source *source);
extern void RICO_audio_source_play_loop(struct RICO_audio_source *source);
extern void RICO_audio_source_pause(struct RICO_audio_source *source);
extern void RICO_audio_source_resume(struct RICO_audio_source *source);
extern void RICO_audio_source_stop(struct RICO_audio_source *source);
extern enum RICO_audio_state RICO_audio_source_state(
    struct RICO_audio_source *source);
extern void RICO_audio_buffer_load_file(struct RICO_audio_buffer *buffer,
                                        const char *filename);
extern void RICO_audio_buffer_load(struct RICO_audio_buffer *buffer, u32 len,
                                   char *data);
extern void RICO_audio_buffer_free(struct RICO_audio_buffer *buffer);

#endif