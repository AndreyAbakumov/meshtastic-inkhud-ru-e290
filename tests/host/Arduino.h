#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
using std::min;
using std::max;
#define PROGMEM
class __FlashStringHelper;
class String : public std::string {
public:
    using std::string::string;
};
inline void yield() {}

uint32_t millis();
