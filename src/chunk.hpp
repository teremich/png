#pragma once

#include "file.hpp"
#include "png.hpp"

bool checkIHDR(const PNG& png);
void loadPLTE(PNG& png);
void loadtRNS(PNG& png);
uint8_t getColorType(const PNG& png);
void getDimensions(const PNG& png, std::uint32_t *width, std::uint32_t *height, std::uint8_t *bit_depth);
[[nodiscard]] allocation_t decompressIDAT(const PNG& png);
