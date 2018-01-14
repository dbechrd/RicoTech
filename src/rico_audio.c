internal ALCdevice *audio_device = 0;
internal ALCcontext *audio_context = 0;
internal ALuint audio_buffer;
ALuint audio_source;

int init_openal()
{
    ALenum err;

    const char *devices = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    const char *s = devices;
    while (*s)
    {
        printf("Device: %s\n", s);
        s += strlen(s) + 1;
    }

    audio_device = alcOpenDevice(NULL);
    if (!audio_device)
        return RICO_FATAL(ERR_OPENAL_INIT, "Failed to open audio device");

    ALCint attrlist[] = { ALC_FREQUENCY, 44100, 0 };
    audio_context = alcCreateContext(audio_device, attrlist);
    if (!audio_context)
        return RICO_FATAL(ERR_OPENAL_INIT, "Failed to create audio context");

    if (!alcMakeContextCurrent(audio_context))
        return RICO_FATAL(ERR_OPENAL_INIT, "Failed to activate audio context");

    alGenSources(1, &audio_source);
    alSourcef(audio_source, AL_PITCH, 1.0f);
    alSourcef(audio_source, AL_GAIN, 1.0f);
    alSourcei(audio_source, AL_LOOPING, AL_TRUE);

    struct vec3 src_pos = VEC3(0.0f, 1.5f, 0.0f);
    alSourcefv(audio_source, AL_POSITION, (float *)&src_pos);
    alSourcefv(audio_source, AL_VELOCITY, (float *)&VEC3_ZERO);

    alGenBuffers(1, &audio_buffer);

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
    u32 len;
    char *buf;
    file_contents("audio/thunder_storm.raw", &len, &buf);
    alBufferData(audio_buffer, AL_FORMAT_MONO16, buf, len, SAMPLE_RATE);
    free(buf);
#endif

    //alSourceQueueBuffers(audio_source, 1, &audio_buffer);
    alSourcei(audio_source, AL_BUFFER, audio_buffer);

    //AL_INITIAL, AL_PLAYING, AL_PAUSED, AL_STOPPED
    //ALenum state;
    //alGetSourcei(audio_source, AL_SOURCE_STATE, &state);

    alSourcePlay(audio_source);

    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);

    return SUCCESS;
}