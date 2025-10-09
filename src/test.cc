#include "png.hpp"

#include <bmp.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

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

static uint32_t image_rgba[]{
    0xFF0000FF, 0xFF000000,
    0x00FF00FF, 0x00FF0000
};

int main() {
    PNG png = createPNG(image_rgba, 2, 2);
    writePNG(png, "image.png");
    return 0;
}
