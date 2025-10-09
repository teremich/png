#include "image.hpp"
#include "chunk.hpp"
#include "file.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>

static uint8_t paethPredictor(std::uint8_t A, std::uint8_t B, std::uint8_t C) {
    int8_t P = A + B - C;
    uint8_t pa = abs(P - A);
    uint8_t pb = abs(P - B);
    uint8_t pc = abs(P - C);
    return (pa <= pb && pa <= pc) ? A : (pb <= pc) ? B : C;
}

byte_t* unfilterScanlines(PNG png, std::uint32_t *width, std::uint32_t *height, std::uint8_t *bit_depth) {
    getDimensions(png, width, height, bit_depth);
    assert(*bit_depth && !(*bit_depth & 7));
    auto data = decompressIDAT(png);
    size_t perPixelSize;
    switch (getColorType(png)) {
        case 3: // indexed-color
            perPixelSize = 1;
            break;
        case 0: // grayscale
            perPixelSize = 1**bit_depth / 8;
            break;
        case 4: // grayscale with alpha
            perPixelSize = 2**bit_depth / 8;
            break;
        case 2: // truecolor
            perPixelSize = 3**bit_depth / 8;
            break;
        case 6: // truecolor with alpha
            perPixelSize = 4**bit_depth / 8;
            break;
        default: 
            perPixelSize = sizeof(PNG::PLTE::Color)+1;
    };
    byte_t* result = static_cast<byte_t*>(
        std::malloc(*width**height**bit_depth*perPixelSize/8)
    );
    const size_t lineSize = *width*perPixelSize**bit_depth/8 + sizeof(FilteredScanline);
    byte_t *null = new byte_t[perPixelSize];
    for (std::uint32_t y = 0; y < *height; y++) {
        FilteredScanline* currentLine = reinterpret_cast<FilteredScanline*>(
            lineSize * y + static_cast<byte_t*>(data.ptr)
        );
        if (reinterpret_cast<byte_t*>(currentLine) + lineSize > data.size+static_cast<byte_t*>(data.ptr)) {
            std::printf("currentLine overflowed data from decompressIDAT\n");
            std::printf("poiting %zd bytes into data\n",
                reinterpret_cast<byte_t*>(currentLine)-static_cast<byte_t*>(data.ptr)
            );
            std::printf("while one line would take up %zu bytes\n", lineSize);
            std::exit(1);
        }
        auto filterType = currentLine->t;
        for (std::uint32_t x = 0; x < *width*perPixelSize; x++) {
            const byte_t
                *C = x && y ? &result[(y-1)**width*perPixelSize+x-perPixelSize] : null,
                *B = y ? &result[(y-1)**width*perPixelSize+x] : null,
                *A = x ? &result[y**width*perPixelSize+x-perPixelSize] : null
            ;
            byte_t *X = &result[y**width*perPixelSize+x];
            const byte_t *Filt = &currentLine->line[x];
            switch(filterType) {
                case 0: // None
                    *X = *Filt;
                    break;
                case 1: // Sub
                    *X = *Filt + *A;
                    break;
                case 2: // Up
                    *X = *Filt + *B;
                    break;
                case 3: // Average
                    *X = *Filt + (*A + *B) / 2;
                    break;
                case 4: // Paeth
                    std::printf("What is wrong with you\n");
                    std::exit(1);
                    *X = paethPredictor(*A, *B, *C);
                    break;
                default:
                    std::printf("unknown filter type\n");
                    std::exit(1);
            }
        }
    }
    free(data.ptr);
    delete[] null;
    return result;
}

pixel_rgba indexToPixel(const PNG& png, std::uint8_t index) {
    return {
        .r = png.palette.colors[index].r,
        .g = png.palette.colors[index].g,
        .b = png.palette.colors[index].b,
        .a = (index < png.palette.numTransparencies) ?
            png.palette.transparencies[index] : static_cast<uint8_t>(0xFF),
    };
}

static uint32_t* loadIndexed(
    uint8_t* data, const PNG& png,
    std::uint32_t width, std::uint32_t height, std::uint8_t bit_depth
) {
    assert(!(bit_depth & 7)); // last 3 bits are 0 so bit_depth divides perfectly into bytes
    uint32_t *result = static_cast<uint32_t*>(malloc(4*width*height*bit_depth/8));
    for (size_t index = 0; index < width*height*bit_depth/8; index++) {
        const auto p = indexToPixel(png, data[index]);
        result[index] =
            ((p.r) << (8*3)) |
            ((p.g) << (8*2)) |
            ((p.b) << (8*1)) |
            ((p.a) << (8*0));
    }
    free(data);
    return result;
}

uint32_t* loadPixels(const PNG& png) {
    std::uint32_t width, height;
    std::uint8_t bit_depth;
    byte_t* data = unfilterScanlines(
        png, &width, &height, &bit_depth
    );
    switch(getColorType(png)) {
        case 3: // indexed color
            return loadIndexed(data, png, width, height, bit_depth);
        case 6: // true color with alpha
            return reinterpret_cast<uint32_t*>(data);
        default: // too lazy to implement the rest
            return reinterpret_cast<uint32_t*>(data);
    }
}

void unloadPixels(uint32_t* pixels) {
    free(pixels);
}
