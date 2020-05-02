#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include <glad/glad.h>

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

struct Texture3D {
    uint32_t id, unit;

    Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t unit) : unit(unit) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_3D, id);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, width, height, depth, 0, GL_RGBA, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_3D, 0);
        glBindImageTexture(unit, id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    }

    void use() {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_3D, id);
    }
};

struct TextureMask3D {
    uint32_t id, unit;

    TextureMask3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t unit) : unit(unit) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_3D, id);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, width, height, depth, 0, GL_RED_INTEGER, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_3D, 0);
        glBindImageTexture(unit, id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
    }

    void use() {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_3D, id);
    }
};

struct Cubemap {
    uint32_t unit, id;

    Cubemap() {}
    Cubemap(std::vector<std::string> filenames, uint32_t unit, bool srgb);

    void use() {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    }
};