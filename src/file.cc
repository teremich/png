#include "file.hpp"
#include <filesystem>
#include <cassert>

[[nodiscard]] allocation_t mapFile(const char* filePath) {
    int file = open(filePath, O_RDONLY);
    const auto fileSize = std::filesystem::file_size(filePath);
    return {mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE | MAP_FILE, file, 0), fileSize};
}

void createFile(const char* filename, std::uint8_t *content, std::size_t size) {
    FILE* const file = fopen(filename, "wb+");
    assert(fwrite(content, size, 1, file) == 1);
    fclose(file);
}

void unmapFile(allocation_t mappedFile) {
    munmap(mappedFile.ptr, mappedFile.size);
}
