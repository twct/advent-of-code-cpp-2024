# Advent of Code 2024

Provides next-day solutions for Advent of Code 2024 challenges in simple, plain C++.

## Build Instructions

Any of the built-in CMake project generators should work:

```sh
$ mkdir build && cd build
$ cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
$ ninja
```

### Running

```sh
$ ./day### <path/to/input/file.txt>
```

The input file is optional. If not specified, the default, publicly available puzzle input is used.
