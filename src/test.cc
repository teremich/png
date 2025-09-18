#include "chunk.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>

int main() {
    PNG png = loadPNG("dartboard.png");
    if (png.data == NULL) {
        return EXIT_FAILURE;
    }
    uint32_t width, height;
    getDimensions(png, &width, &height);
    unloadPNG(png);
    printf("width: %u, height: %u\n", width, height);
    return 0;
}
