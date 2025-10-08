#include "chunk.hpp"
#include "file.hpp"
#include "png.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <algorithm>

#include <zlib.h>

struct [[gnu::packed]] IHDR {
    enum CompressionMethods{
        METHOD_ZERO = 0
    };
    std::uint32_t width, height;
    std::uint8_t bit_depth, color_type, compression_method, filter_method, interlace_method;
    crc_t CRC;
};

typedef byte_t IDAT;

struct sRGB{
    crc_t CRC;
};

bool checkIHDR(PNG png) {
    Chunk* firstChunk = png.data->chunks;
    if ((uintptr_t)firstChunk >= (uintptr_t)png.data+png.totalSize-sizeof(IHDR)) {
        return false;
    }
    bool correct = true;
    correct = correct && (firstChunk->chunk_type == Chunk::IHDR);
    correct = correct && ((IHDR*)firstChunk->chunkdata_and_crc)->compression_method == IHDR::METHOD_ZERO;
    correct = correct && (__builtin_bswap32(firstChunk->length) == sizeof(IHDR)-sizeof(crc_t));
    return correct;
}

[[nodiscard]] allocation_t decompressIDAT(PNG png) {
    union {
        Chunk* chunk;
        byte_t* address;
    } current = {.address = reinterpret_cast<byte_t*>(png.data->chunks)};
    byte_t* end = current.address + png.totalSize;
    for (
        ;
        current.address + Chunk::minSize + __builtin_bswap32(current.chunk->length) < end;
        current.address += Chunk::minSize + __builtin_bswap32(current.chunk->length)
    ) {
        std::printf("processing chunk: %.4s\n", (char*)&current.chunk->chunk_type);
        if (
            current.chunk->chunk_type == Chunk::IHDR ||
            current.chunk->chunk_type == Chunk::PLTE
        ) {
            continue;
        }
        if (current.chunk->chunk_type == Chunk::IEND) {
            return {};
        }
        if (current.chunk->chunk_type == Chunk::IDAT) {
            break;
        }
        if (!(current.chunk->chunk_type & Chunk::NON_CRITICAL_BIT)) {
            std::fprintf(stderr, "%x %x %x %x\n", ((char*)&current.chunk->chunk_type)[0], ((char*)&current.chunk->chunk_type)[1], ((char*)&current.chunk->chunk_type)[2], ((char*)&current.chunk->chunk_type)[3]);
            std::fprintf(stderr, "couldn't process critical Chunk: %.4s\n", (char*)&current.chunk->chunk_type);
            std::exit(1);
        }
    }
    allocation_t image_data{};
    static const char* version = zlibVersion();
    const size_t zOutputSize = 1024;
    byte_t *const zOutput = static_cast<byte_t*>(std::malloc(zOutputSize));
    // The fields
    // next_in, avail_in, zalloc, zfree and opaque
    // must be initialized before by the caller.
    z_stream stream{
        .next_in = NULL,    // no data ready yet
        .avail_in = 0,      // 
        .zalloc = nullptr,  // use default allocation function
        .zfree = nullptr,   // use default free function
        .opaque = nullptr,  // optional data for this stream
    };
    assert(inflateInit(&stream) == Z_OK);
    std::printf("initialized inflate stream\n");
    while (
        current.address < end - (Chunk::minSize + __builtin_bswap32(current.chunk->length))
        && current.chunk->chunk_type == Chunk::IDAT
    ) {
        std::printf("idat chunk: %.4s\n", (char*)&current.chunk->chunk_type);
        IDAT* idat = current.chunk->chunkdata_and_crc;
        std::uint32_t length = __builtin_bswap32(current.chunk->length);
        assert(stream.avail_in == 0);
        stream.next_in = idat;
        stream.avail_in = std::min<std::ptrdiff_t>(length, end-sizeof(crc_t)-current.chunk->chunkdata_and_crc);

        stream.next_out = zOutput;
        stream.avail_out = zOutputSize;

        std::printf("adding %u = std::min<std::ptrdiff_t>(%u, %zu) to stream\n", stream.avail_in, length, end-sizeof(crc_t)-current.chunk->chunkdata_and_crc);
        std::printf("inflate: %d\n", inflate(&stream, Z_SYNC_FLUSH));
        const size_t written = zOutputSize - stream.avail_out;
        std::printf("getting %zu bytes from stream\n", written);
        std::printf("prev size: %zu\n", image_data.size);
        image_data.size += written;
        std::printf("next size: %zu\n", image_data.size);

        image_data.ptr = std::realloc(image_data.ptr, image_data.size);
        std::memcpy(
            static_cast<byte_t*>(image_data.ptr) + image_data.size-written,
            zOutput,
            written
        );
        current.address += Chunk::minSize + length;
    }
    if (current.address < end - Chunk::minSize) {
        std::printf("stopped processing IDAT because we found following chunk type: %.4s\n", (char*)&current.chunk->chunk_type);
    }
    inflate(&stream, Z_FINISH);
    for (int i = 0; i < 50 && stream.avail_in; i++) {
        stream.next_out = zOutput;
        stream.avail_out = zOutputSize;
        std::printf("inflate: %d\n", inflate(&stream, Z_SYNC_FLUSH));
        const size_t written = zOutputSize - stream.avail_out;
        std::printf("getting %zu bytes from stream\n", written);
        std::printf("prev size: %zu\n", image_data.size);
        image_data.size += written;
        std::printf("next size: %zu\n", image_data.size);
        image_data.ptr = std::realloc(image_data.ptr, image_data.size);
        std::memcpy(
            static_cast<byte_t*>(image_data.ptr) + image_data.size-written,
            zOutput,
            stream.next_out - zOutput
        );
        std::printf("there are still %u bytes available after flush\n", stream.avail_in);
    }
    std::free(zOutput);
    if (stream.avail_in) {
        std::printf("there are still %u bytes available after flush\n", stream.avail_in);
        std::exit(1);
    }
    inflateEnd(&stream);
    std::printf("ended inflate stream\n");
    return image_data;
}

void getDimensions(PNG png, std::uint32_t *width, std::uint32_t *height) {
    Chunk* firstChunk = png.data->chunks;
    IHDR* header = reinterpret_cast<IHDR*>(firstChunk->chunkdata_and_crc);
    *width = __builtin_bswap32(header->width);
    *height = __builtin_bswap32(header->height);
}
