#pragma once
#include <cstdlib>
#include <cstdint>

typedef uint8_t byte_t;
typedef byte_t data_t[];
typedef uint32_t crc_t;

constexpr char tmp[] = "sRGB";
constexpr char tmp0 = tmp[0];
constexpr char tmp1 = tmp[1];
constexpr char tmp2 = tmp[2];
constexpr char tmp3 = tmp[3];

struct [[gnu::packed]] Chunk{
    uint32_t length;
    enum : uint32_t {
        IHDR = 0x52'44'48'49,
        IDAT = 0x54'41'44'49,
        IEND = 0x44'4e'54'49,
        sRGB = 0x42'47'52'73,
        tRNS = 0x53'4e'52'74,
    } chunk_type;
    data_t chunkdata_and_crc;
};

//                            len+hdr+(chks+crc)
static_assert(sizeof(Chunk) == 4 + 4 + 0);

struct PNG_datastream{
    byte_t signature[8];
    struct Chunk chunks[];
};

struct PNG{
    size_t totalSize;
    PNG_datastream* data;
};

const PNG loadPNG(const char* filename);
void unloadPNG(PNG png);