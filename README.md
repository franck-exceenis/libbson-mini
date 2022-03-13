# libbson-mini

This project is a lightweight bson manipulation library.

It doesn't implement the whole standard, but allows you to use bson format even in very constrained/embedded systems.

# Integration in a project

You only have to use this as a git submodule and use directly the `bson.h` and `bson.c` files.
The whole C project only uses libc headers, so it's easy to integrate it in any platform/OS
(linux, windows, FreeRTOS, ESP32, STM32...).

You can also use the C++ API by also adding `bson.hpp` and `bson.cpp` files.
The whole C++ project only uses STL library, so it's still easy to integrate.
