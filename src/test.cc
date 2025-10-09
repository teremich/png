#include "chunk.hpp"
#include "image.hpp"
#include "png.hpp"

#include <bmp.h>

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

static void RGBA2ARGB(std::uint32_t* pixels, std::size_t num) {
    for (std::size_t i = 0; i < num; i++) {
        const uint8_t alpha = pixels[i] & 0xFF;
        pixels[i] = (alpha << (3*8)) | (pixels[i] >> 8);
    }
}

static std::uint32_t* PNG_PALETTE2ARGB(PNG& png) {
    std::uint32_t* out = static_cast<std::uint32_t*>(std::malloc(
        sizeof(std::uint32_t) * png.palette.numColors
    ));
    for (std::size_t i = 0; i < png.palette.numTransparencies; i++) {
        out[i] =
            png.palette.transparencies[i] << (3*8) |
            png.palette.colors[i].r << (2*8) |
            png.palette.colors[i].g << (1*8) |
            png.palette.colors[i].b << (0*8)
        ;
    }
    for (std::size_t i = png.palette.numTransparencies; i < png.palette.numColors; i++) {
        out[i] =
            (0xFF << (3*8)) |
            png.palette.colors[i].r << (2*8) |
            png.palette.colors[i].g << (1*8) |
            png.palette.colors[i].b << (0*8)
        ;
    }
    return out;
}

int main() {
    PNG png = loadPNG("dartboard.png");
    if (png.data == NULL) {
        std::printf("file couldn't be loaded, possibly corrupted\n");
        return EXIT_FAILURE;
    }
    std::uint32_t width, height;
    std::uint8_t bit_depth;
    getDimensions(png, &width, &height, &bit_depth);
    uint32_t* pixels = loadPixels(png);
    if (pixels == NULL) {
        std::printf("pixels couldn't be loaded, possibly corrupted\n");
        return EXIT_FAILURE;
    }
    assert(bit_depth == 8);

    // BMP bmp{
    //     .width = 2,
    //     .height = 2,
    //     .pixels = {.packed = test_argb}
    // };

    // writeBMP(bmp, "test_argb.bmp");

    // RGBA2ARGB(test_rgba, 4);

    // bmp.pixels = {.packed = test_rgba};

    // writeBMP(bmp, "test_rgba.bmp");

    RGBA2ARGB(pixels, static_cast<std::size_t>(width)*height);

    uint32_t *palette = PNG_PALETTE2ARGB(png);

    BMP bmp{
        .width = static_cast<uint32_t>(png.palette.numColors),
        .height = 1,
        .pixels = {.packed = palette},
    };

    writeBMP(bmp, "dartboard-palette.bmp");

    bmp = {
        width, height, {.packed = pixels}
    };

    writeBMP(bmp, "dartboard.bmp");

    free(palette);

    unloadPixels(pixels);
    unloadPNG(png);
    return 0;
}
