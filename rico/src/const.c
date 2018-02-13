const char *rico_error_string[] = {
    RICO_ERRORS(GEN_STRING)
};

void string_truncate(char *buf, int buf_count, int length)
{
    if (buf_count < 16)
    {
        RICO_ASSERT(0);  // Why are you truncating such a tiny buffer!?
        return;
    }

    if (length > buf_count)
    {
        char *buf_end = buf + (buf_count - 4);
        *buf_end = '.'; buf_end++;
        *buf_end = '.'; buf_end++;
        *buf_end = '.';
    }
}

enum rico_error rico_error_print(const char *file, int line,
                                 enum rico_error err, const char *fmt, ...)
{
    if (err != SUCCESS)
    {
        char desc[256] = { 0 };
        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(desc, sizeof(desc), fmt, args);
        va_end(args);

        string_truncate(desc, sizeof(desc), len);

        fprintf(stderr, "[ERROR][%s:%d][%d:%s] %s\n", strstr(file, "src"), line,
                err, rico_error_string[err], desc);
    }
    return err;
}

enum rico_error rico_fatal_print(const char *file, int line,
                                 enum rico_error err, const char *fmt, ...)
{
    if (err != SUCCESS)
    {
        RICO_ASSERT(RICO_DEBUG_FATAL_ASSERT == 0);

        char desc[256] = { 0 };
        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(desc, sizeof(desc), fmt, args);
        string_truncate(desc, sizeof(desc), len);
        va_end(args);

        fprintf(stderr, "[FATAL][%s:%d][%d:%s] %s\n", strstr(file, "src"), line,
                err, rico_error_string[err], desc);

        char title[128] = { 0 };
        len = snprintf(title, sizeof(title), "%s : Line %d",
                       strstr(file, "src"), line);
        string_truncate(title, sizeof(title), len);

        char msg[500] = { 0 };
        snprintf(msg, sizeof(msg), "%s [%d]\n\n%s", rico_error_string[err], err,
                 desc);
        string_truncate(msg, sizeof(msg), len);

        fflush(stdout);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, NULL);
    }
    return err;
}
