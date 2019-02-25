#ifndef RICO_AUDIO_H
#define RICO_AUDIO_H

extern float ric_audio_volume();
extern void ric_audio_volume_set(float volume);
extern bool ric_audio_muted();
extern void ric_audio_mute();
extern void ric_audio_unmute();
extern void ric_audio_toggle();
extern void ric_audio_source_init(struct ric_audio_source *source);
extern void ric_audio_source_free(struct ric_audio_source *source);
extern void ric_audio_source_buffer(struct ric_audio_source *source,
                                    struct ric_audio_buffer *buffer);
extern void ric_audio_source_pitch_set(struct ric_audio_source *source,
                                       float pitch);
extern void ric_audio_source_gain_set(struct ric_audio_source *source,
                                      float gain);
extern void ric_audio_source_play(struct ric_audio_source *source);
extern void ric_audio_source_play_loop(struct ric_audio_source *source);
extern void ric_audio_source_pause(struct ric_audio_source *source);
extern void ric_audio_source_resume(struct ric_audio_source *source);
extern void ric_audio_source_stop(struct ric_audio_source *source);
extern enum ric_audio_state ric_audio_source_state(struct ric_audio_source
                                                   *source);
extern void ric_audio_buffer_load_file(struct ric_audio_buffer *buffer,
                                       const char *filename);
extern void ric_audio_buffer_load(struct ric_audio_buffer *buffer, char *data,
                                  u32 len);
extern void ric_audio_buffer_free(struct ric_audio_buffer *buffer);

#endif