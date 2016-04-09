#ifndef UTIL_H
#define UTIL_H

void *file_contents(const char *filename, int *length);
void *read_tga(const char *filename, int *width, int *height);

#endif