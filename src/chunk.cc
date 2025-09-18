#include "chunk.hpp"
#include "png.hpp"
#include <cstdlib>

struct IHDR{

};

struct sRGB{
    
};

void getDimensions(PNG png, uint32_t *width, uint32_t *height) {
    const byte_t* current = reinterpret_cast<const byte_t*>(png.data->chunks);
    while (true) {
        const Chunk* chunk = reinterpret_cast<const Chunk*>(current);
        switch(chunk->chunk_type) {
            case Chunk::IHDR: {
                const IHDR* header = reinterpret_cast<const IHDR*>(chunk->chunkdata_and_crc);

                } break;
            default:
                break;
        }
        current += sizeof(*chunk) + chunk->length + sizeof(crc_t);
    }
}
