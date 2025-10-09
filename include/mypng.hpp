#pragma once
#include <cstdlib>
#include <cstdint>

typedef std::uint8_t byte_t;
typedef byte_t data_t[];
typedef std::uint32_t crc_t;

struct PNG_datastream;

struct PNG{
    std::size_t totalSize;
    PNG_datastream* data;
    struct ColorPalette{
        size_t numColors;
        struct Color{
            uint8_t r, g, b;
        } *colors;
        size_t numTransparencies;
        uint8_t* transparencies;
    } palette;
};

const PNG loadPNG(const char* filename);
std::uint32_t* loadPixels(const PNG& png);
PNG createPNG(std::uint32_t* pixels, std::uint32_t width, std::uint32_t height);
void writePNG(const PNG& png, const char* filename);
void unloadPixels(std::uint32_t* pixels);
void unloadPNG(PNG png);
