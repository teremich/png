#pragma once

#include <cstdlib>
#include <cstdint>
#include <sys/mman.h>
#include <sys/file.h>

typedef struct Mapping{
    const void* ptr;
    std::size_t size;
} mapping_t;

typedef struct Allocation{
    void* ptr;
    std::size_t size;
} allocation_t;

[[nodiscard]] allocation_t mapFile(const char* filePath);
void createFile(const char* filename, std::uint8_t *content, std::size_t size);
void unmapFile(allocation_t mappedFile);

