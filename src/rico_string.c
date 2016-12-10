// #include "rico_string.h"
// #include <string.h>

// #define POOL_SIZE 50

// static struct rico_string pool[POOL_SIZE];
// static u32 next_handle = 1;

// struct rico_string string_init(const char *str, u32 length)
// {
//     //TODO: Handle out-of-memory
//     //TODO: Implement reuse of pool objects
//     if (next_handle >= POOL_SIZE)
//     {
//         fprintf(stderr, "Out of memory: String pool exceeded max size of %d.\n",
//                 POOL_SIZE);
//         return NULL;
//     }

//     struct rico_string *str = &pool[next_handle];
//     next_handle++;

//     str->length = length;
//     strncpy(str->name, name, length);
// }