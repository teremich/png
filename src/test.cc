#include "chunk.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>

struct Allocation{
    static inline const auto deallocate = free;
    void* ptr;
    size_t size;
};

[[nodiscard]] struct Allocation readIDAT(PNG png);

// void printData(Allocation data) {
//     for (int i = 0; i < data.size; i++) {
//         printf("0x%02X ", static_cast<byte_t*>(data.ptr)[i]);
//         if ((i % 8)-8 == -1) {
//             printf("\t");
//         }
//         if ((i % 16)-16 == -1) {
//             printf("\n");
//         }
//     }
// }

int main() {
    PNG png = loadPNG("dartboard.png");
    if (png.data == NULL) {
        printf("file couldn't be loaded, possibly corrupted\n");
        return EXIT_FAILURE;
    }
    uint32_t width, height;
    getDimensions(png, &width, &height);
    const auto image_data = readIDAT(png);

    // printData(image_data);

    image_data.deallocate(image_data.ptr);
    unloadPNG(png);
    printf("width: %u, height: %u\n", width, height);
    return 0;
}
