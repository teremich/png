#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct pixel_bgra{
    uint8_t b, g, r, a;
};

struct BMP {
    uint32_t width, height;
    union{
        struct pixel_bgra *argb;
        uint32_t *packed;
    } pixels;
};

void writeBMP(struct BMP bitmap, const char* filename);


#ifdef __cplusplus
}; // extern "C"
#endif
