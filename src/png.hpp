#pragma once
#include <mypng.hpp>

struct [[gnu::packed]] Chunk{
    std::uint32_t length;
    enum : std::uint32_t {
        NON_CRITICAL_BIT = 1<<5,
        IHDR = 0x52'44'48'49,
        IDAT = 0x54'41'44'49,
        PLTE = 0x45'54'4c'50,
        IEND = 0x44'4e'45'49,
        tRNS = 0x53'4e'52'74,
        cHRM = 0x4d'52'58'63,
        gAMA = 0x41'4d'41'67,
        iCCP = 0x50'43'43'69,
        sBIT = 0x54'49'42'73,
        sRGB = 0x42'47'52'73,
        cICP = 0x50'43'49'63,
        mDCV = 0x56'43'44'6D,
        cLLI = 0x49'4C'4C'63,
        tEXt = 0x74'58'45'74,
        zTXt = 0x74'58'54'7A,
        iTXt = 0x74'58'54'69,
        bKGD = 0x44'47'4B'62,
        hIST = 0x54'53'49'68,
        pHYs = 0x73'59'48'70,
        sPLT = 0x54'4C'50'73,
        eXIf = 0x66'49'58'65,
        tIME = 0x45'4D'49'74,
        acTL = 0x4C'54'63'61,
        fcTL = 0x4C'54'63'66,
        fdAT = 0x54'41'64'66
    } chunk_type;
    byte_t chunkdata_and_crc[];
    static const std::size_t minSize;
};

inline constexpr std::size_t Chunk::minSize = sizeof(Chunk) + sizeof(crc_t);

//                            len+hdr+(chnk+crc)
static_assert(sizeof(Chunk) == 4 + 4 + 0);

struct PNG_datastream{
    byte_t signature[8];
    struct Chunk chunks[];
};
