#pragma once

#include <string>
#include <stdint.h>

struct Texture {
    uint32_t id, unit;

    Texture(std::string path, uint32_t unit, bool alpha);
    void use(); 
};