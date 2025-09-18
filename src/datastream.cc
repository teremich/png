#include "file.hpp"
#include "png.hpp"
#include <cstring>

const byte_t PNG_MAGIC_BYTES[] {
    0x89, 0x50, 0x4E, 0x47,
    0x0D, 0x0A, 0x1A, 0x0A
};

bool checkHeader(const PNG_datastream* data) {
    return !memcmp(data->signature, PNG_MAGIC_BYTES, sizeof(PNG_MAGIC_BYTES));
}

const PNG loadPNG(const char* filename) {
    const auto file = mapFile(filename);
    auto *datastream = static_cast<PNG_datastream*>(file.ptr);
    if (checkHeader(datastream)) {
        return {file.size, datastream};
    } else {
        unmapFile(file);
        return {0, NULL};
    }
}

void unloadPNG(PNG png) {
    const allocation_t file = {png.data, png.totalSize};
    unmapFile(file);
}
