#pragma once
#include <cstdlib>
#include <cstdint>

typedef std::uint8_t byte_t;
typedef byte_t data_t[];
typedef std::uint32_t crc_t;

struct [[gnu::packed]] Chunk{
    std::uint32_t length;
    enum : std::uint32_t {
        NON_CRITICAL_BIT = 1<<5,
        IHDR = 0x52'44'48'49,
        IDAT = 0x54'41'44'49,
        PLTE = 0x45'54'4c'50,
        IEND = 0x44'4e'54'49,
        sRGB = 0x42'47'52'73,
        tRNS = 0x53'4e'52'74,
    } chunk_type;
    data_t chunkdata_and_crc;
    static const std::size_t minSize;
};

inline constexpr std::size_t Chunk::minSize = sizeof(Chunk) + sizeof(crc_t);

//                            len+hdr+(chnk+crc)
static_assert(sizeof(Chunk) == 4 + 4 + 0);

struct PNG_datastream{
    byte_t signature[8];
    struct Chunk chunks[];
};

struct PNG{
    std::size_t totalSize;
    PNG_datastream* data;
    struct PLTE{
        size_t num;
        struct Color{
            uint8_t r, g, b;
        } *colors;
    } palette;
};

const PNG loadPNG(const char* filename);
uint32_t* loadPixels(const PNG& png);

void unloadPixels(uint32_t* pixels);
void unloadPNG(PNG png);