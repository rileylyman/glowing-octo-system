#pragma once

#include <string>
#include <stdint.h>

struct Texture {
    uint32_t id, unit;

    Texture(): id(UINT32_MAX), unit(UINT32_MAX) {}
    Texture(std::string filename, uint32_t unit);
    void use(); 
};