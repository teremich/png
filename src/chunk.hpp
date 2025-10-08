#pragma once

#include "png.hpp"

bool checkIHDR(PNG png);
void getDimensions(PNG data, std::uint32_t* width, std::uint32_t* height);
