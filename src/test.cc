#include "chunk.hpp"
#include <mypng.hpp>
#define BMP_IMPLEMENTATION
#include <bmp.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

static void ABGR2ARGB(std::uint32_t* pixels, std::size_t num) {
    for (std::size_t i = 0; i < num; i++) {
        uint8_t r = pixels[i];
        uint8_t b = pixels[i] >> (2*8);
        pixels[i] = pixels[i] & 0xFF00FF00 | (r << (2*8)) | b;
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

static uint32_t image_abgr[]{
    0xFF0000FF, 0x000000FF,
    0xFF00FF00, 0x0000FF00
};

int main() {
    // =========== W PNG ============
    PNG png = createPNG(image_abgr, 2, 2);
    writePNG(png, "image.png");
    free(png.data);
    // =========== W PNG ============
    // =========== IMAGE ============
    png = loadPNG("image.png");
    uint32_t width, height;
    getDimensions(png, &width, &height, nullptr);
    uint32_t* pixels = loadPixels(png);
    ABGR2ARGB(pixels, width*height);
    BMP bmp{
        width, height, {.packed=pixels}
    };
    writeBMP(bmp, "image.bmp");
    unloadPixels(pixels);
    unloadPNG(png);
    // =========== IMAGE ============
    // =========== DARTS ============
    png = loadPNG("dartboard.png");
    getDimensions(png, &width, &height, nullptr);
    pixels = loadPixels(png);
    ABGR2ARGB(pixels, width*height);
    bmp = {
        width, height, {.packed=pixels}
    };
    writeBMP(bmp, "dartboard.bmp");
    unloadPixels(pixels);
    unloadPNG(png);
    // =========== DARTS ============
    return 0;
}
