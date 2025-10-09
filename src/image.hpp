#pragma once

#include "png.hpp"
#include <cstdint>

struct FilteredScanline{
    static constexpr std::uint8_t filterMethod = 0;
    std::uint8_t t;
    byte_t line[]; // indices or sample values
};

struct pixel_rgba{
    uint8_t r, g, b, a;
};

byte_t* unfilterScanlines(PNG png, std::uint32_t *width, std::uint32_t *height, std::uint8_t *bit_depth);
pixel_rgba indexToPixel(const PNG& png, std::uint8_t index);
