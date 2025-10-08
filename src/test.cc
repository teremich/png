#include "image.hpp"
#include "png.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

// #define RAW_FORMATTING

void printData(const PNG& png, std::uint8_t* data, std::size_t size) {
    std::printf("allocation size: %zu\n", size);
    for (int i = 0; i < size; i++) {
#       ifndef RAW_FORMATTING
            auto p = indexToPixel(png, data[i]);
            std::printf("%02X%02X%02X%02X ", p.r, p.g, p.b, 255);
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
            std::printf("%c", data[i]);
#       endif
    }
    if (size % 16) {
        std::printf("\n");
    }
}

int main() {
    PNG png = loadPNG("dartboard.png");
    if (png.data == NULL) {
        std::printf("file couldn't be loaded, possibly corrupted\n");
        return EXIT_FAILURE;
    }
    uint32_t* pixels = loadPixels(png);

    

    unloadPixels(pixels);
    unloadPNG(png);
    return 0;
}
