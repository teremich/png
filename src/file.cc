#include "file.hpp"
#include <filesystem>

[[nodiscard]] allocation_t mapFile(const char* filePath) {
    int file = open(filePath, O_RDONLY);
    const auto fileSize = std::filesystem::file_size(filePath);
    return {mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE | MAP_FILE, file, 0), fileSize};
}

void unmapFile(allocation_t mappedFile) {
    munmap(mappedFile.ptr, mappedFile.size);
}
