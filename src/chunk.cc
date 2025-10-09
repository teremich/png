#include "chunk.hpp"
#include "file.hpp"
#include "png.hpp"
#include "crc.hpp"
#include "image.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <algorithm>

#include <zconf.h>
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
    byte_t* const end = current.address + png.totalSize;
    for (
        ;
        current.address + Chunk::minSize + __builtin_bswap32(current.chunk->length) < end;
        current.address += Chunk::minSize + __builtin_bswap32(current.chunk->length)
    ) {
        if (current.chunk->chunk_type == Chunk::PLTE) {
            goto found;
        }
        if (current.chunk->chunk_type == Chunk::IEND) {
            return;
        }
    }
    return;
found:
    png.palette.colors = reinterpret_cast<PNG::PLTE::Color*>(current.chunk->chunkdata_and_crc);
    png.palette.numColors = __builtin_bswap32(current.chunk->length)/sizeof(PNG::PLTE::Color);
    std::uint8_t bit_depth;
    getDimensions(png, 0, 0, &bit_depth);
    if (png.palette.numColors > 1<<bit_depth) {
        std::printf(
            "Error: the number of colors in the palette (%zu) exceeded the bit_depth\n",
            png.palette.numColors
        );
        png.palette.numColors = 1<<bit_depth;
    }
    std::printf("color palette of size %zu loaded\n", png.palette.numColors);
}

void loadtRNS(PNG& png) {
    union {
        Chunk* chunk;
        byte_t* address;
    } current = {.address = reinterpret_cast<byte_t*>(png.data->chunks)};
    byte_t* const end = current.address + png.totalSize;
    for (
        ;
        current.address + Chunk::minSize + __builtin_bswap32(current.chunk->length) < end;
        current.address += Chunk::minSize + __builtin_bswap32(current.chunk->length)
    ) {
        if (current.chunk->chunk_type == Chunk::tRNS) {
            goto found;
        }
        if (current.chunk->chunk_type == Chunk::IEND) {
            return;
        }
    }
    return;
found:
    png.palette.transparencies = reinterpret_cast<uint8_t*>(current.chunk->chunkdata_and_crc);
    png.palette.numTransparencies = __builtin_bswap32(current.chunk->length);
    if (png.palette.numTransparencies > png.palette.numColors) {
        std::printf(
            "Error: the number of transparency values "
            "in the palette (%zu) exceeded the number of colors (%zu)\n",
            png.palette.numTransparencies,
            png.palette.numColors
        );
        png.palette.numTransparencies = png.palette.numColors;
    }
    std::printf("transparency palette of size %zu loaded\n", png.palette.numTransparencies);
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
        if (current.chunk->chunk_type == Chunk::tRNS) {
            std::printf("tRNS chunk has length: %u\n", __builtin_bswap32(current.chunk->length));
        }
    }
    allocation_t image_data{};
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

[[nodiscard]] static allocation_t compressIDAT(allocation_t data) {
    const size_t zOutputSize = 1<<14;
    allocation_t compressed_data{
        std::malloc(zOutputSize),
        zOutputSize
    };
    // The fields
    // next_in, avail_in, zalloc, zfree and opaque
    // must be initialized before by the caller.
    z_stream stream{
        .next_in = static_cast<Bytef*>(data.ptr),    // no data ready yet
        .avail_in = static_cast<uint32_t>(data.size),      // 
        .zalloc = nullptr,  // use default allocation function
        .zfree = nullptr,   // use default free function
        .opaque = nullptr,  // optional data for this stream
    };
    stream.next_out = static_cast<Bytef*>(compressed_data.ptr);
    stream.avail_out = zOutputSize;
    assert(
        deflateInit(
            &stream, Z_DEFAULT_COMPRESSION
        ) == Z_OK
    );
    while (true) {
        // const std::uint32_t before = stream.avail_out;
        int r = deflate(&stream, Z_FINISH);
        if (r == Z_STREAM_END) {
            compressed_data.size -= stream.avail_out;
            break;
        }
        assert(r == Z_OK);
        // const std::size_t written = before - stream.avail_out;
        stream.avail_out = compressed_data.size / 2;
        compressed_data.size += stream.avail_out;
        compressed_data.ptr = std::realloc(compressed_data.ptr, compressed_data.size);
        stream.next_out = &static_cast<byte_t*>(
            compressed_data.ptr
        )[compressed_data.size - stream.avail_out];
    }
    deflateEnd(&stream);
    std::printf("we compressed %zu bytes of image data into %zu bytes\n", data.size, compressed_data.size);
    return compressed_data;
}

void getDimensions(const PNG& png, std::uint32_t *width, std::uint32_t *height, std::uint8_t *bit_depth) {
    Chunk* firstChunk = png.data->chunks;
    IHDR* header = reinterpret_cast<IHDR*>(firstChunk->chunkdata_and_crc);
    if (width) {
        *width = __builtin_bswap32(header->width);
    }
    if (height) {
        *height = __builtin_bswap32(header->height);
    }
    if (bit_depth) {
        *bit_depth = header->bit_depth;
    }
    assert(*bit_depth == 8);
    assert(header->interlace_method == 0); // no interlace
    // const char* name;
    // switch(header->color_type) {
    //     case 0:
    //         name = "grayscale";
    //         break;
    //     case 2:
    //         name = "true color";
    //         break;
    //     case 3:
    //         name = "indexed color";
    //         break;
    //     case 4:
    //         name = "grayscale with alpha";
    //         break;
    //     case 6:
    //         name = "true color with alpha";
    //         break;
    //     default:
    //         name = "unkown";
    // }
}

void createIHDR(PNG& png, std::uint32_t width, std::uint32_t height) {
    png.totalSize = sizeof(PNG_datastream) + sizeof(Chunk) + sizeof(IHDR);
    png.data = static_cast<PNG_datastream*>(
        std::malloc(png.totalSize)
    );
    png.data->chunks[0].length = __builtin_bswap32(sizeof(IHDR) - sizeof(crc_t));
    png.data->chunks[0].chunk_type = Chunk::IHDR;
    IHDR* header = reinterpret_cast<IHDR*>(png.data->chunks[0].chunkdata_and_crc);
    *header = {
        .width = __builtin_bswap32(width),
        .height = __builtin_bswap32(height),
        .bit_depth = 8,
        .color_type = 6,
        .compression_method = 0,
        .filter_method = 0,
        .interlace_method = 0
    };
    header->CRC = crc(
        reinterpret_cast<Bytef*>(&png.data->chunks[0].chunk_type),
        sizeof(Chunk::chunk_type) + sizeof(IHDR) - sizeof(crc_t)
    );
}

void createIDAT(PNG& png, std::uint32_t* pixel_data) {
    std::uint32_t width, height;
    std::uint8_t bit_depth;
    getDimensions(png, &width, &height, &bit_depth);
    // interlace method 0 => no interlacing
    const auto filtered = filterScanlines(pixel_data, width, height);
    const allocation_t IDAT = compressIDAT(filtered);
    std::free(filtered.ptr);
    
    std::size_t oldSize = png.totalSize;
    png.totalSize += Chunk::minSize;
    png.totalSize += IDAT.size;
    png.data = static_cast<PNG_datastream*>(realloc(png.data, png.totalSize));
    void* start_of_IDAT_chunk = &reinterpret_cast<byte_t*>(png.data)[oldSize];
    Chunk* idat = static_cast<Chunk*>(start_of_IDAT_chunk);
    idat->length = __builtin_bswap32(static_cast<std::uint32_t>(IDAT.size));
    idat->chunk_type = Chunk::IDAT;
    std::size_t data_to_crc_size = IDAT.size + sizeof(Chunk::chunk_type);
    byte_t* data_to_crc = reinterpret_cast<byte_t*>(
        &idat->chunkdata_and_crc
    );
    byte_t* CRC = &data_to_crc[IDAT.size];
    *reinterpret_cast<crc_t*>(CRC) = crc(data_to_crc, data_to_crc_size);
    std::memcpy(
        static_cast<Chunk*>(start_of_IDAT_chunk)->chunkdata_and_crc,
        IDAT.ptr, IDAT.size
    );
    std::free(IDAT.ptr);
}

void createIEND(PNG& png) {
    std::size_t begin_of_last_chunk = png.totalSize;
    png.totalSize += Chunk::minSize;
    png.data = static_cast<PNG_datastream*>(realloc(png.data, png.totalSize));
    byte_t* last_chunk = &reinterpret_cast<byte_t*>(png.data)[begin_of_last_chunk];
    Chunk* IEND = reinterpret_cast<Chunk*>(last_chunk);
    IEND->chunk_type = Chunk::IEND;
    IEND->length = 0;
    crc_t* CRC = reinterpret_cast<crc_t*>(&IEND->chunkdata_and_crc);
    *CRC = crc(reinterpret_cast<byte_t*>(&IEND->chunk_type), sizeof(Chunk::chunk_type));
}
