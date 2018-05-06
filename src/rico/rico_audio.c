const char *RICO_audio_state_string[] = { RICO_AUDIO_STATE(GEN_STRING) };

static ALCdevice *audio_device = 0;
static ALCcontext *audio_context = 0;

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
		RICO_FATAL(ERR_OPENAL_INIT, "Failed to open audio device");
		return;
	}

    ALCint attrlist[] = { ALC_FREQUENCY, 44100, 0 };
    audio_context = alcCreateContext(audio_device, attrlist);
	if (!audio_context)
	{
		RICO_FATAL(ERR_OPENAL_INIT, "Failed to create audio context");
		return;
	}

	if (!alcMakeContextCurrent(audio_context))
	{
		RICO_FATAL(ERR_OPENAL_INIT, "Failed to activate audio context");
		return;
	}

    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);
}

extern float RICO_audio_volume()
{
    ALfloat volume = 0;
    alGetListenerf(AL_GAIN, &volume);
    return volume;
}
extern void RICO_audio_volume_set(float volume)
{
    alListenerf(AL_GAIN, volume);
}
extern void RICO_audio_mute()
{
    RICO_audio_volume_set(0.0f);
}
extern void RICO_audio_unmute()
{
    RICO_audio_volume_set(1.0f);
}
extern void RICO_audio_source_init(struct RICO_audio_source *source)
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
extern void RICO_audio_source_free(struct RICO_audio_source *source)
{
    alDeleteSources(1, &source->al_source_id);
}
extern void RICO_audio_buffer_load_file(struct RICO_audio_buffer *buffer,
                                        const char *filename)
{
    u32 len;
    char *data;
    file_contents(filename, &len, &data);
    RICO_audio_buffer_load(buffer, len, data);
    free(data);
}
#if 0
// TODO: Copy rico_mesh auto-load scheme for audio buffers that that are loaded
//       in from packs.
static void audio_upload(struct ral_audio_buffer *buffer)
{

}
#endif
extern void RICO_audio_source_play(struct RICO_audio_source *source)
{
    source->loop = false;
    alSourcei(source->al_source_id, AL_LOOPING, source->loop);
    alSourcePlay(source->al_source_id);
}
extern void RICO_audio_source_play_loop(struct RICO_audio_source *source)
{
    source->loop = true;
    alSourcei(source->al_source_id, AL_LOOPING, source->loop);
    alSourcePlay(source->al_source_id);
    printf("Playing source id=%d pitch=%f gain=%f loop=%d\n",
           source->al_source_id, source->pitch, source->gain, source->loop);
}
extern void RICO_audio_source_pause(struct RICO_audio_source *source)
{
    alSourcePause(source->al_source_id);
}
extern void RICO_audio_source_resume(struct RICO_audio_source *source)
{
    alSourcePlay(source->al_source_id);
}
extern void RICO_audio_source_stop(struct RICO_audio_source *source)
{
    alSourceStop(source->al_source_id);
}
extern enum RICO_audio_state RICO_audio_source_state(struct RICO_audio_source *source)
{
    ALenum state;
    alGetSourcei(source->al_source_id, AL_SOURCE_STATE, &state);

    switch (state)
    {
        case AL_INITIAL:
        case AL_STOPPED:
            return RICO_AUDIO_STOPPED;
        case AL_PLAYING:
            return RICO_AUDIO_STOPPED;
        case AL_PAUSED:
            return RICO_AUDIO_STOPPED;
        default:
            return RICO_AUDIO_UNKNOWN;
    }
}
extern void RICO_audio_buffer_load(struct RICO_audio_buffer *buffer, u32 len,
                            char *data)
{
    alGenBuffers(1, &buffer->al_buffer_id);

#define SAMPLE_RATE 44100
#if 0
    // TODO: Load from file
    const u32 AMPLITUDE = 500;
    s16 buf[SAMPLE_RATE];

    //const double TWO_PI = 6.28318;
    const double ring1 = 80.0 / SAMPLE_RATE; //350
    const double ring2 = 180.0 / SAMPLE_RATE; //440
    double x1 = 0;
    double x2 = 0;

    for (unsigned i = 0; i < SAMPLE_RATE; ++i)
    {
        buf[i] = (s16)(AMPLITUDE / 2 * (sin(x1 * M_2PI) + sin(x2 * M_2PI)));
        x1 += ring1;
        x2 += ring2;
    }
    alBufferData(audio_buffer, AL_FORMAT_MONO16, buf, sizeof(buf), SAMPLE_RATE);
#else
    // TODO: Allow caller to specify format and sample rate
    alBufferData(buffer->al_buffer_id, AL_FORMAT_MONO16, data, len, SAMPLE_RATE);
#endif
}
extern void RICO_audio_source_buffer(struct RICO_audio_source *source,
                              struct RICO_audio_buffer *buffer)
{
    //alSourceQueueBuffers(audio_source, 1, &audio_buffer);
    alSourcei(source->al_source_id, AL_BUFFER, buffer->al_buffer_id);
}
extern void RICO_audio_buffer_free(struct RICO_audio_buffer *buffer)
{
    alDeleteBuffers(1, &buffer->al_buffer_id);
}
