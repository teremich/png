#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#ifdef BMP_IMPLEMENTATION
#include <stdio.h>
#endif

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

#define BM 0x4D42
#define DIB_HEADER_SIZE 40

struct __attribute__((packed)) bmp_header {
    uint16_t magic_bytes;
    uint32_t size_in_bytes;
    uint32_t reserved;
    uint32_t image_data_offset;
    struct dib_header{
        uint32_t dib_header_size;
        uint32_t width_in_pixels;
        int32_t height_in_pixels;
        uint16_t num_color_planes;
        uint16_t bpp;
        uint32_t compression;
        uint32_t bitmap_data_padded_size; // may be 0 if compression is 0
        int32_t horizontal_pixels_per_meter;
        int32_t vertical_pixels_per_meter;
        uint32_t num_palette_colors;
        uint32_t num_important_colors;
    } dib;
    uint8_t pixel_data[];
};

void writeBMP(struct BMP bitmap, const char* filename)
#ifdef BMP_IMPLEMENTATION
{
    FILE* file = fopen(filename, "w+");
    size_t pixel_data_size = bitmap.width*bitmap.height*4;
    size_t size = pixel_data_size + sizeof(struct bmp_header);
    struct bmp_header header = {
        BM, (uint32_t)size, 0,
        sizeof(struct bmp_header),
        {
            DIB_HEADER_SIZE,
            bitmap.width, (int32_t)-bitmap.height,
            1, 4*8, 0,
            (uint32_t)pixel_data_size,
            2835, 2835,
            0, 0
        }
    };

    fwrite(&header, sizeof(struct bmp_header), 1, file);
    for (uint32_t y = 0; y < bitmap.height; y++) {
        fwrite(
            &bitmap.pixels.packed[y*bitmap.width],
            sizeof(uint32_t), bitmap.width, file
        );
    }
    fclose(file);
}
#else
;
#endif

#ifdef __cplusplus
}; // extern "C"
#endif
