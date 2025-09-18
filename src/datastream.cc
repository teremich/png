#include "chunk.hpp"
#include "file.hpp"
#include "png.hpp"
#include <cstring>

const byte_t PNG_MAGIC_BYTES[] {
    0x89, 0x50, 0x4E, 0x47,
    0x0D, 0x0A, 0x1A, 0x0A
};

bool checkMagicBytes(PNG png) {
    return !memcmp(png.data->signature, PNG_MAGIC_BYTES, sizeof(PNG_MAGIC_BYTES));
}

bool checkHeader(PNG png) {
    if (png.totalSize < sizeof(PNG_MAGIC_BYTES) + sizeof(Chunk) + sizeof(crc_t)) {
        return false;
    }
    return checkMagicBytes(png) && checkIHDR(png);
}

const PNG loadPNG(const char* filename) {
    const auto file = mapFile(filename);
    auto *const datastream = static_cast<PNG_datastream*>(file.ptr);
    const PNG png = {file.size, datastream};
    if (checkHeader(png)) {
        return png;
    } else {
        unmapFile(file);
        return {0, NULL};
    }
}

void unloadPNG(PNG png) {
    const allocation_t file = {png.data, png.totalSize};
    unmapFile(file);
}
