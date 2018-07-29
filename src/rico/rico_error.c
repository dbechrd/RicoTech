static enum ric_error rico_error_print(const char *file, int line,
                                        enum ric_error err, const char *fmt,
                                        ...)
{
    if (err != RIC_SUCCESS)
    {
        char desc[256] = { 0 };
        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(desc, sizeof(desc), fmt, args);
        va_end(args);

        string_truncate(desc, sizeof(desc), len);

        fprintf(stderr, "[ERROR][%s:%d][%d:%s] %s\n", strstr(file, "src"), line,
                err, ric_error_string[err], desc);
    }
    return err;
}
static enum ric_error rico_fatal_print(const char *file, int line,
                                        enum ric_error err, const char *fmt,
                                        ...)
{
    if (err != RIC_SUCCESS)
    {
        RICO_ASSERT(RICO_DEBUG_FATAL_ASSERT == 0);

        char desc[256] = { 0 };
        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(desc, sizeof(desc), fmt, args);
        string_truncate(desc, sizeof(desc), len);
        va_end(args);

        fprintf(stderr, "[FATAL][%s:%d][%d:%s] %s\n", strstr(file, "src"), line,
                err, ric_error_string[err], desc);

        char title[128] = { 0 };
        len = snprintf(title, sizeof(title), "%s : Line %d",
                       strstr(file, "src"), line);
        string_truncate(title, sizeof(title), len);

        char msg[500] = { 0 };
        snprintf(msg, sizeof(msg), "%s [%d]\n\n%s", ric_error_string[err], err,
                 desc);
        string_truncate(msg, sizeof(msg), len);

        fflush(stdout);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, NULL);
    }
    return err;
}