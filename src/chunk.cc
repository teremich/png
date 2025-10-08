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

bool checkIHDR(const PNG& png) {
    Chunk* firstChunk = png.data->chunks;
    if ((uintptr_t)firstChunk >= (uintptr_t)png.data+png.totalSize-sizeof(IHDR)) {
        return false;
    }
    bool correct = true;
    correct = correct && (firstChunk->chunk_type == Chunk::IHDR);
    correct = correct && (reinterpret_cast<const IHDR*>(
        firstChunk->chunkdata_and_crc
        )->compression_method == IHDR::METHOD_ZERO);
    correct = correct && (reinterpret_cast<const IHDR*>(
        firstChunk->chunkdata_and_crc
        )->filter_method == 0);
    correct = correct && (__builtin_bswap32(firstChunk->length) == sizeof(IHDR)-sizeof(crc_t));
    return correct;
}

uint8_t getColorType(const PNG& png) {
    const Chunk* const firstChunk = png.data->chunks;
    const IHDR* const header = reinterpret_cast<const IHDR*>(firstChunk->chunkdata_and_crc);
    return header->color_type;
}

void loadPLTE(PNG& png) {
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
        if (current.chunk->chunk_type == Chunk::PLTE) {
            break;
        }
        if (current.chunk->chunk_type == Chunk::IEND) {
            return;
        }
    }
    // TODO: what if there is no IEND chunk?
    png.palette.colors = reinterpret_cast<PNG::PLTE::Color*>(current.chunk->chunkdata_and_crc);
    png.palette.num = current.chunk->length/sizeof(PNG::PLTE::Color);
    std::printf("color palette loaded\n");
}

[[nodiscard]] allocation_t decompressIDAT(const PNG& png) {
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
    const size_t zOutputSize = 1<<14;
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
    while (
        current.address < end - (Chunk::minSize + __builtin_bswap32(current.chunk->length))
        && current.chunk->chunk_type == Chunk::IDAT
    ) {
        IDAT* idat = current.chunk->chunkdata_and_crc;
        std::uint32_t length = __builtin_bswap32(current.chunk->length);
        assert(stream.avail_in == 0);
        stream.next_in = idat;
        stream.avail_in = std::min<std::ptrdiff_t>(length, end-sizeof(crc_t)-current.chunk->chunkdata_and_crc);

        stream.next_out = zOutput;
        stream.avail_out = zOutputSize;

        assert(inflate(&stream, Z_SYNC_FLUSH) == Z_OK);
        const size_t written = zOutputSize - stream.avail_out;
        image_data.size += written;

        image_data.ptr = std::realloc(image_data.ptr, image_data.size);
        std::memcpy(
            static_cast<byte_t*>(image_data.ptr) + image_data.size-written,
            zOutput,
            written
        );
        current.address += Chunk::minSize + length;
    }
    inflate(&stream, Z_FINISH);
    for (int i = 0; i < 50 && stream.avail_in; i++) {
        stream.next_out = zOutput;
        stream.avail_out = zOutputSize;
        int r = inflate(&stream, Z_SYNC_FLUSH);
        if (r == Z_STREAM_END) {
            assert(stream.avail_in == 0);
        } else {
            assert(r == Z_OK);
        }
        const size_t written = zOutputSize - stream.avail_out;
        image_data.size += written;
        image_data.ptr = std::realloc(image_data.ptr, image_data.size);
        std::memcpy(
            static_cast<byte_t*>(image_data.ptr) + image_data.size-written,
            zOutput,
            written
        );
    }
    std::free(zOutput);
    if (stream.avail_in) {
        std::exit(1);
    }
    inflateEnd(&stream);
    std::printf("we received %zu bytes of image data\n", image_data.size);
    return image_data;
}

void getDimensions(const PNG& png, std::uint32_t *width, std::uint32_t *height, std::uint8_t *bit_depth) {
    Chunk* firstChunk = png.data->chunks;
    IHDR* header = reinterpret_cast<IHDR*>(firstChunk->chunkdata_and_crc);
    *width = __builtin_bswap32(header->width);
    *height = __builtin_bswap32(header->height);
    *bit_depth = header->bit_depth;
    assert(*bit_depth == 8);
    assert(header->interlace_method == 0); // no interlace
    std::printf("color type: ");
    const char* name;
    switch(header->color_type) {
        case 0:
            name = "grayscale";
            break;
        case 2:
            name = "true color";
            break;
        case 3:
            name = "indexed color";
            break;
        case 4:
            name = "grayscale with alpha";
            break;
        case 6:
            name = "true color with alpha";
            break;
        default:
            name = "unkown";
    }
    printf("%s\n", name);
}
