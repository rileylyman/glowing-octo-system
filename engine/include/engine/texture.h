#pragma once

#include <vector>
#include <string>
#include <stdint.h>

enum TextureType {
    TEXTURE_TYPE_NORMAL_MAP = 0,
    TEXTURE_TYPE_ALBEDO_MAP,
    TEXTURE_TYPE_METALLIC_MAP,
    TEXTURE_TYPE_ROUGHNESS_MAP,
    TEXTURE_TYPE_METALLIC_ROUGHNESS_MAP,
    TEXTURE_TYPE_HEIGHT_MAP,
    TEXTURE_TYPE_AMBIENT_MAP,
    TEXTURE_TYPE_DIFFUSE_MAP,
    TEXTURE_TYPE_SPECULAR_MAP,
    TEXTURE_TYPE_AO_MAP,
};

struct Texture {
    uint32_t id, unit;

    Texture(): id(UINT32_MAX), unit(UINT32_MAX) {}
    Texture(std::string filename, uint32_t unit, bool srgb);
    void use(); 
};

struct Cubemap : public Texture {
    Cubemap(std::vector<std::string> filenames, uint32_t unit, bool srgb);
};