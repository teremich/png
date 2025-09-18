#include "chunk.hpp"
#include "png.hpp"
#include <cstdlib>
#include <cassert>

struct [[gnu::packed]] IHDR {
    uint32_t width, height;
    uint8_t bit_depth, color_type, compression_method, filter_method, interlace_method;
    crc_t CRC;
};

struct sRGB{
    crc_t CRC;
};

bool checkIHDR(PNG png) {
    Chunk* firstChunk = png.data->chunks;
    bool correct = true;
    correct = correct && (firstChunk->chunk_type == Chunk::IHDR);
    correct = correct && (__builtin_bswap32(firstChunk->length) == sizeof(IHDR)-sizeof(crc_t));
    return correct;
}

void getDimensions(PNG png, uint32_t *width, uint32_t *height) {
    Chunk* firstChunk = png.data->chunks;
    IHDR* header = reinterpret_cast<IHDR*>(firstChunk->chunkdata_and_crc);
    *width = __builtin_bswap32(header->width);
    *height = __builtin_bswap32(header->height);
}
