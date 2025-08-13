disclaimer: only have tested on MacOS arm64.

build:
`cmake -B build -S .`
`cmake --build build`

run:
`imgtoascii -i [path to input file] -s [downscale factor]`

The downscale factor determines the pixel per character ratio. If you specify a downscale factor of 5, for example, each character in the final ascii image will represent a 5x5 block of pixels.
