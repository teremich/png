#include "file.hpp"
#include <filesystem>

[[nodiscard]] allocation_t mapFile(const char* filePath) {
    int file = open(filePath, O_RDONLY);
    const auto fileSize = std::filesystem::file_size(filePath);
    return {mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE | MAP_FILE, file, 0), fileSize};
}

std::size_t createFile(const char* filename, std::uint8_t *content, std::size_t size) {
    FILE* const file = fopen(filename, "wb+");
    const std::size_t written = fwrite(content, size, 1, file);
    fclose(file);
    return written;
}

void unmapFile(allocation_t mappedFile) {
    munmap(mappedFile.ptr, mappedFile.size);
}
