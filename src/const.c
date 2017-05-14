const char *rico_error_string[] = {
    RICO_ERRORS(GEN_STRING)
};

enum rico_error rico_error_print(const char *file, int line,
                                 enum rico_error err, const char *fmt, ...)
{
    if (err != SUCCESS)
    {
        char desc[256] = { 0 };
        va_list args;
        va_start(args, fmt);
        vsnprintf(desc, sizeof(desc), fmt, args);
        va_end(args);

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
        vsnprintf(desc, sizeof(desc), fmt, args);
        va_end(args);

        fprintf(stderr, "[FATAL][%s:%d][%d:%s] %s\n", strstr(file, "src"), line,
                err, rico_error_string[err], desc);

        char title[128] = { 0 };
        snprintf(title, sizeof(title), "%s : Line %d", strstr(file, "src"),
                 line);

        char msg[500] = { 0 };
        snprintf(msg, sizeof(msg), "%s [%d]\n\n%s", rico_error_string[err], err,
                 desc);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, NULL);
    }
    return err;
}