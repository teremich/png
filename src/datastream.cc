#include "chunk.hpp"
#include "file.hpp"
#include "png.hpp"

#include <cstring>
#include <cstdio>

const byte_t PNG_MAGIC_BYTES[] {
    0x89, 0x50, 0x4E, 0x47,
    0x0D, 0x0A, 0x1A, 0x0A
};

bool checkMagicBytes(PNG png) {
    return !std::memcmp(png.data->signature, PNG_MAGIC_BYTES, sizeof(PNG_MAGIC_BYTES));
}

bool checkHeader(PNG png) {
    if (png.totalSize < sizeof(PNG_MAGIC_BYTES) + Chunk::minSize) {
        return false;
    }
    return checkMagicBytes(png) && checkIHDR(png);
}

PNG findHeader(allocation_t file) {
    for (std::size_t i = 0; i < file.size; i++) {
        auto *const datastream = static_cast<PNG_datastream*>(file.ptr);
        PNG png = {file.size-i, datastream};
        if (checkHeader(png)) {
            return png;
        }
    }
    return {};
}

const PNG loadPNG(const char* filename) {
    const auto file = mapFile(filename);
    PNG png = findHeader(file);
    if (png.totalSize) {
    } else {
        unmapFile(file);
        return {0, NULL};
    }
    std::uint8_t color_type = getColorType(png);
    if (color_type == 3) { // we could optionally include color types 2 and 6 here
        loadPLTE(png);
    } else {
        std::printf("no color palette available\n");
    }
    return png;
}

void unloadPNG(PNG png) {
    const allocation_t file = {png.data, png.totalSize};
    unmapFile(file);
}
