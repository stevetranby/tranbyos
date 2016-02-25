#include "include/system.h"

typedef struct {
    const char* bytes;
    u32 count;
} String;

typedef struct {
    void* elements;
    u32 size;
} DynArray;

enum class StringError;

StringError k_strncpy();
