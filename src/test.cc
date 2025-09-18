#include "chunk.hpp"
#include <cstdint>

int main() {
    PNG png = loadPNG("dartboard.png");
    uint32_t width, height;
    getDimensions(png, &width, &height);
    unloadPNG(png);
    return width + height;
}
