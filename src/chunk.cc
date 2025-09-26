#include "chunk.hpp"
#include "file.hpp"
#include "png.hpp"
#include <cstdlib>
#include <cassert>
#include <cstring>

struct [[gnu::packed]] IHDR {
    uint32_t width, height;
    uint8_t bit_depth, color_type, compression_method, filter_method, interlace_method;
    crc_t CRC;
};

typedef byte_t IDAT;

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

[[nodiscard]] allocation_t readIDAT(PNG png) {
    byte_t* address = reinterpret_cast<byte_t*>(png.data->chunks);
    byte_t* end = address + png.totalSize;
    Chunk* currentChunk;
    for (
        currentChunk = reinterpret_cast<Chunk*>(address);
        address + sizeof(Chunk) + __builtin_bswap32(currentChunk->length) + sizeof(crc_t) < end;
        address += sizeof(Chunk) + __builtin_bswap32(currentChunk->length) + sizeof(crc_t)
    ) {
        currentChunk = reinterpret_cast<Chunk*>(address);
        if (currentChunk->chunk_type == Chunk::IDAT) {
            break;
        }
    }
    allocation_t image_data{};
    do {
        IDAT* idat = currentChunk->chunkdata_and_crc;
        uint32_t length = __builtin_bswap32(currentChunk->length);
        image_data.size += length;
        image_data.ptr = realloc(image_data.ptr, image_data.size);
        std::memcpy(
            static_cast<byte_t*>(image_data.ptr) + image_data.size-length,
            idat,
            length
        );

        address += sizeof(Chunk) + length + sizeof(crc_t);
        currentChunk = reinterpret_cast<Chunk*>(address);
    }
    while (address < end && currentChunk->chunk_type == Chunk::IDAT);
    return image_data;
}

void getDimensions(PNG png, uint32_t *width, uint32_t *height) {
    Chunk* firstChunk = png.data->chunks;
    IHDR* header = reinterpret_cast<IHDR*>(firstChunk->chunkdata_and_crc);
    *width = __builtin_bswap32(header->width);
    *height = __builtin_bswap32(header->height);
}
