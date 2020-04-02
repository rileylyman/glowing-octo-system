#pragma once

#include <vector>
#include <string>
#include <stdint.h>

struct Texture {
    uint32_t id, unit;

    Texture(): id(UINT32_MAX), unit(UINT32_MAX) {}
    Texture(std::string filename, uint32_t unit, bool srgb);
    void use(); 
};

struct Cubemap : public Texture {
    Cubemap(std::vector<std::string> filenames, uint32_t unit, bool srgb);
};