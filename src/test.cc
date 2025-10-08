#include "chunk.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>

struct Allocation{
    static inline const auto deallocate = std::free;
    void* ptr;
    std::size_t size;
};

[[nodiscard]] Allocation decompressIDAT(PNG png);

void printData(Allocation data) {
    std::printf("allocation size: %zu\n", data.size);
    for (int i = 0; i < data.size; i++) {
        std::printf("0x%02X ", static_cast<byte_t*>(data.ptr)[i]);
        if ((i % 8) == 8-1) {
            std::printf("\t");
        }
        if ((i % 16) == 16-1) {
            std::printf("\n");
        }
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
    const auto image_data = decompressIDAT(png);

    printData(image_data);

    image_data.deallocate(image_data.ptr);
    unloadPNG(png);
    std::printf("width: %u, height: %u\n", width, height);
    return 0;
}
