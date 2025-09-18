#pragma once

#include "png.hpp"

bool checkIHDR(PNG png);
void getDimensions(PNG data, uint32_t* width, uint32_t* height);
