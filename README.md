this project has only been tested on MacOS arm64 and MSVC14 on Windows.

build:
cmake (unix)
`cmake -B build -S .`
`cmake --build build`

visual studio (windows)
open sln file and right click build all

run:
`imgtoascii -i [path to input file] -s [downscale factor]`

The downscale factor determines the pixel per character ratio. If you specify a downscale factor of 5, for example, each character in the final ascii image will represent a 5x5 block of pixels.

there is a single included file in the "resources" folder for testing the program
