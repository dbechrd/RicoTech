static ALCdevice *audio_device = 0;
static ALCcontext *audio_context = 0;

static ALfloat global_audio_volume = 1.0f;
static bool global_audio_muted = false;

static void init_openal()
{
    ALenum err;

    const char *devices = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    const char *s = devices;
    while (*s)
    {
        printf("Device: %s\n", s);
        s += dlb_strlen(s) + 1;
    }

    audio_device = alcOpenDevice(NULL);
	if (!audio_device)
	{
		RICO_FATAL(RIC_ERR_OPENAL_INIT, "Failed to open audio device");
		return;
	}

    ALCint attrlist[] = { ALC_FREQUENCY, 44100, 0 };
    audio_context = alcCreateContext(audio_device, attrlist);
	if (!audio_context)
	{
		RICO_FATAL(RIC_ERR_OPENAL_INIT, "Failed to create audio context");
		return;
	}

	if (!alcMakeContextCurrent(audio_context))
	{
		RICO_FATAL(RIC_ERR_OPENAL_INIT, "Failed to activate audio context");
		return;
	}

    err = alGetError();
    if (err) RICO_ERROR(RIC_ERR_OPENAL_INIT, "OpenAL error: %s\n", err);
}

extern float ric_audio_volume()
{
    //ALfloat volume = 0;
    //alGetListenerf(AL_GAIN, &volume);
    //return volume;
    return global_audio_volume;
}
extern void ric_audio_volume_set(float volume)
{
    global_audio_volume = volume;
    alListenerf(AL_GAIN, global_audio_volume);
}
extern bool ric_audio_muted()
{
    return global_audio_muted;
}
extern void ric_audio_mute()
{
    global_audio_muted = true;
    alListenerf(AL_GAIN, 0.0f);
}
extern void ric_audio_unmute()
{
    global_audio_muted = false;
    ric_audio_volume_set(global_audio_volume);
}
extern void ric_audio_toggle()
{
    global_audio_muted ? ric_audio_unmute() : ric_audio_mute();
}
extern void ric_audio_source_init(struct ric_audio_source *source)
{
    if (!source->pitch) source->pitch = 1.0f;
    if (!source->gain) source->gain = 1.0f;

    alGenSources(1, &source->al_source_id);
    alSourcef(source->al_source_id, AL_PITCH, source->pitch);
    alSourcef(source->al_source_id, AL_GAIN, source->gain);

    // TODO: Attach rico_audio to objects which auto-update this stuff
#ifdef POSITIONAL_SOUND
    struct vec3 src_pos = VEC3(0.0f, 1.5f, 0.0f);
    alSourcefv(audio_source, AL_POSITION, (float *)&src_pos);
    alSourcefv(audio_source, AL_VELOCITY, (float *)&VEC3_ZERO);
#else
    alSourcei(source->al_source_id, AL_SOURCE_RELATIVE, AL_TRUE);
    //alSourcefv(audio_source, AL_POSITION, (float *)&VEC3_ZERO);
    //alSourcefv(audio_source, AL_POSITION, (float *)&VEC3_ZERO);
#endif
}
extern void ric_audio_source_free(struct ric_audio_source *source)
{
    alDeleteSources(1, &source->al_source_id);
}
extern void ric_audio_buffer_load_file(struct ric_audio_buffer *buffer,
                                       const char *filename)
{
    char *data;
    u32 len;
    file_contents(filename, &data, &len);
    ric_audio_buffer_load(buffer, data, len);
    free(data);
}
#if 0
// TODO: Copy rico_mesh auto-load scheme for audio buffers that that are loaded
//       in from packs.
static void audio_upload(struct ral_audio_buffer *buffer)
{

}
#endif
extern void ric_audio_source_pitch_set(struct ric_audio_source *source, float pitch)
{
    source->pitch = pitch;
    alSourcef(source->al_source_id, AL_PITCH, source->pitch);
}
extern void ric_audio_source_gain_set(struct ric_audio_source *source, float gain)
{
    source->gain = gain;
    alSourcef(source->al_source_id, AL_GAIN, source->gain);
}
extern void ric_audio_source_play(struct ric_audio_source *source)
{
    source->loop = false;
    alSourcei(source->al_source_id, AL_LOOPING, source->loop);
    alSourcePlay(source->al_source_id);
}
extern void ric_audio_source_play_loop(struct ric_audio_source *source)
{
    source->loop = true;
    alSourcei(source->al_source_id, AL_LOOPING, source->loop);
    alSourcePlay(source->al_source_id);
    printf("Playing source id=%d pitch=%f gain=%f loop=%d\n",
           source->al_source_id, source->pitch, source->gain, source->loop);
}
extern void ric_audio_source_pause(struct ric_audio_source *source)
{
    alSourcePause(source->al_source_id);
}
extern void ric_audio_source_resume(struct ric_audio_source *source)
{
    alSourcePlay(source->al_source_id);
}
extern void ric_audio_source_stop(struct ric_audio_source *source)
{
    alSourceStop(source->al_source_id);
}
extern enum ric_audio_state ric_audio_source_state(struct ric_audio_source *source)
{
    ALenum state;
    alGetSourcei(source->al_source_id, AL_SOURCE_STATE, &state);

    switch (state)
    {
        case AL_INITIAL:
        case AL_STOPPED:
            return RIC_AUDIO_STOPPED;
        case AL_PLAYING:
            return RIC_AUDIO_STOPPED;
        case AL_PAUSED:
            return RIC_AUDIO_STOPPED;
        default:
            return RIC_AUDIO_UNKNOWN;
    }
}
extern void ric_audio_buffer_load(struct ric_audio_buffer *buffer, char *data,
                                  u32 len)
{
    alGenBuffers(1, &buffer->al_buffer_id);

#define SAMPLE_RATE 44100
#if 0
    // TODO: Load from file
    const u32 AMPLITUDE = 500;
    s16 buf[SAMPLE_RATE * 10];

    //const double TWO_PI = 6.28318;
    const double ring1 = 80.0 / SAMPLE_RATE; //350
    const double ring2 = 180.0 / SAMPLE_RATE; //440
    double x1 = 0;
    double x2 = 0;

    for (unsigned i = 0; i < ARRAY_COUNT(buf); ++i)
    {
        buf[i] = (s16)(AMPLITUDE / 2 * (sin(x1 * M_2PI) + sin(x2 * M_2PI)));
        x1 += ring1;
        x2 += ring2;
    }
    alBufferData(buffer->al_buffer_id, AL_FORMAT_MONO16, buf, sizeof(buf), SAMPLE_RATE);
#else
    // TODO: Allow caller to specify format and sample rate
    alBufferData(buffer->al_buffer_id, AL_FORMAT_MONO16, data, len, SAMPLE_RATE);
#endif
}
extern void ric_audio_source_buffer(struct ric_audio_source *source,
                                    struct ric_audio_buffer *buffer)
{
    //alSourceQueueBuffers(audio_source, 1, &audio_buffer);
    alSourcei(source->al_source_id, AL_BUFFER, buffer->al_buffer_id);
}
extern void ric_audio_buffer_free(struct ric_audio_buffer *buffer)
{
    alDeleteBuffers(1, &buffer->al_buffer_id);
}
