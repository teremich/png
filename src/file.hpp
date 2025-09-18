#pragma once

#include <sys/mman.h>
#include <sys/file.h>

typedef struct Mapping{
    const void* ptr;
    size_t size;
} mapping_t;

typedef struct Allocation{
    void* ptr;
    size_t size;
} allocation_t;

[[nodiscard]] allocation_t mapFile(const char* filePath);

void unmapFile(allocation_t mappedFile);

