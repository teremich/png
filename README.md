# PNG library

This is a library to convert between PNG files and 32 bit RGBA pixel arrays.

There is also a simple header that writes BGRA bytes to a BMP image.
For swizzeling the pixel data there is a helper function.

I focused on avoiding out of bounds reads by keeping the file size in relevant structs
and not trusting "user input" in this case PNG data.

## Getting Started
- `cmake -S . -B build`
- `cmake --build build`
- `bin/test`

## Resources
[Specification](https://www.w3.org/TR/png-3/#13Decoders)

## Next Steps
- check the CRC when loading an image
