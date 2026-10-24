#pragma once
#include <cstddef>
#include <cstdint>
class Print {
public:
    virtual ~Print() = default;
    virtual size_t write(uint8_t c) = 0;
    size_t print(char c) { return write(static_cast<uint8_t>(c)); }
    size_t print(const char *s) {
        size_t n = 0;
        while (*s) n += write(static_cast<uint8_t>(*s++));
        return n;
    }
};
