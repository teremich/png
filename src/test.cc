#include "chunk.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>

// #define RAW_FORMATTING

struct Allocation{
    static inline const auto deallocate = std::free;
    void* ptr;
    std::size_t size;
};

[[nodiscard]] Allocation decompressIDAT(PNG png);

void printData(Allocation data) {
    std::printf("allocation size: %zu\n", data.size);
    for (int i = 0; i < data.size; i++) {
#       ifndef RAW_FORMATTING
            std::printf("%02X ", static_cast<byte_t*>(data.ptr)[i]);
            // if ((i%4) == 4-1) {
            //     std::printf(" ");
            // }
            if ((i % 8) == 8-1) {
                std::printf(" ");
            }
            if ((i % 16) == 16-1) {
                std::printf("\n");
            }
#       else
            std::printf("%c", static_cast<byte_t*>(data.ptr)[i]);
#       endif
    }
    if (data.size % 16) {
        std::printf("\n");
    }
}

int main() {
    PNG png = loadPNG("dartboard.png");
    if (png.data == NULL) {
        std::printf("file couldn't be loaded, possibly corrupted\n");
        return EXIT_FAILURE;
    }
    std::uint32_t width, height;
    getDimensions(png, &width, &height);
    std::printf("width: %u, height: %u\n", width, height);
    const auto image_data = decompressIDAT(png);

    printData(image_data);

    image_data.deallocate(image_data.ptr);
    unloadPNG(png);
    return 0;
}
