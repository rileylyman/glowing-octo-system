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
    uint32_t width, height, depth;

    Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t unit, GLint sampling_type=GL_LINEAR) : unit(unit), width(width), height(height), depth(depth) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_3D, id);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, sampling_type); 
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, sampling_type); 

        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, width, height, depth, 0, GL_RGBA, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_3D, 0);
    }

    Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t unit, std::vector<float> data, GLint sampling_type=GL_LINEAR) : unit(unit), width(width), height(height), depth(depth) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_3D, id);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, sampling_type); 
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, sampling_type); 

        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, width, height, depth, 0, GL_RGBA, GL_FLOAT, data.data());

        glBindTexture(GL_TEXTURE_3D, 0);
    }

    void use() {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_3D, id);
        glBindImageTexture(unit, id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
    }

    void use(uint32_t tex_unit, uint32_t img_unit) {
        glActiveTexture(GL_TEXTURE0 + tex_unit);
        glBindTexture(GL_TEXTURE_3D, id);
        glBindImageTexture(img_unit, id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
    }

    static std::vector<float> u(uint32_t width, uint32_t height, uint32_t depth) {
        std::vector<float> data;
        for (uint32_t h = 0; h < height; h++) {
            for (uint32_t d = 0; d < depth; d++) {
                for (uint32_t w = 0; w < width; w++) {
                    data.push_back(10.0f);
                    data.push_back(10.0f);
                    data.push_back(10.0f);
                    data.push_back(1.0f);
                }
            }
        }
        return data;
    }
    
    static std::vector<float> q(uint32_t width, uint32_t height, uint32_t depth) {
        std::vector<float> data;
        for (uint32_t h = 0; h < height; h++) {
            for (uint32_t d = 0; d < depth; d++) {
                for (uint32_t w = 0; w < width; w++) {
                    float x = (float)w / (float)width;
                    float y = (float)h / (float)height;
                    float z = (float)d / (float)depth;

                    data.push_back(x);
                    data.push_back(y);
                    data.push_back(z);
                    data.push_back(1.0f);
                }
            }
        }
        return data;
    }

    static std::vector<float> pressure(uint32_t width, uint32_t height, uint32_t depth) {
        std::vector<float> data;
        for (uint32_t h = 0; h < height; h++) {
            for (uint32_t d = 0; d < depth; d++) {
                for (uint32_t w = 0; w < width; w++) {
                    data.push_back(0.0f);
                    data.push_back(0.0f);
                    data.push_back(0.0f);
                    data.push_back(1.0f);
                }
            }
        }
        return data;
    }

    static std::vector<float> world_mask(uint32_t width, uint32_t height, uint32_t depth) {
        std::vector<float> data;
        for (uint32_t h = 0; h < height; h++) {
            for (uint32_t d = 0; d < depth; d++) {
                for (uint32_t w = 0; w < width; w++) {
                    if (h == 0 || h == height - 1 || d == 0 || d == depth - 1 || w == 0 || w == width - 1) {
                        // Solid around boundaries
                        data.push_back(0.0f);
                        data.push_back(0.0f);
                        data.push_back(0.0f);
                        data.push_back(1.0f);
                    } else if ((h >= (height * 2) / 5 && h <= (height * 3) / 5) &&
                               (w >= (width * 2) / 5 && w <= (width * 3) / 5) &&
                               (d >= (depth * 2) / 5 && d <= (depth * 3) / 5)) {
                        // Box in the center
                        data.push_back(0.0f);
                        data.push_back(0.0f);
                        data.push_back(0.0f);
                        data.push_back(1.0f);
                    } else if ((h >= (height * 3) / 10 && h <= (height * 7) / 10) &&
                               (w >= (width * 3) / 10 && w <= (width * 7) / 10) &&
                               (d >= (depth * 3) / 10 && d <= (depth * 7) / 10)) {
                        // Water around the box
                        data.push_back(2.0f);
                        data.push_back(0.0f);
                        data.push_back(0.0f);
                        data.push_back(1.0f);
                    } else {
                        // Water on the Periphery
                        data.push_back(2.0f);
                        data.push_back(0.0f);
                        data.push_back(0.0f);
                        data.push_back(1.0f);
                    }
                }
            }
        }
        return data;
    }

    static std::vector<float> zero(uint32_t width, uint32_t height, uint32_t depth) {
        std::vector<float> data;
        for (uint32_t h = 0; h < height; h++) {
            for (uint32_t d = 0; d < depth; d++) {
                for (uint32_t w = 0; w < width; w++) {
                    data.push_back(0.0f);
                    data.push_back(0.0f);
                    data.push_back(0.0f);
                    data.push_back(0.0f);
                }
            }
        }
        return data;
    }

    static std::vector<float> one(uint32_t width, uint32_t height, uint32_t depth) {
        std::vector<float> data;
        for (uint32_t h = 0; h < height; h++) {
            for (uint32_t d = 0; d < depth; d++) {
                for (uint32_t w = 0; w < width; w++) {
                    data.push_back(1.0f);
                    data.push_back(1.0f);
                    data.push_back(1.0f);
                    data.push_back(0.0f);
                }
            }
        }
        return data;
    }

    static std::vector<float> two(uint32_t width, uint32_t height, uint32_t depth) {
        std::vector<float> data;
        for (uint32_t h = 0; h < height; h++) {
            for (uint32_t d = 0; d < depth; d++) {
                for (uint32_t w = 0; w < width; w++) {
                    data.push_back(2.0f);
                    data.push_back(2.0f);
                    data.push_back(2.0f);
                    data.push_back(1.0f);
                }
            }
        }
        return data;
    }

    static std::vector<float> forces(uint32_t width, uint32_t height, uint32_t depth) {
        std::vector<float> data;
        for (uint32_t h = 0; h < height; h++) {
            for (uint32_t d = 0; d < depth; d++) {
                for (uint32_t w = 0; w < width; w++) {
                    data.push_back(0.0f);
                    data.push_back(0.0f);
                    data.push_back(0.0f);
                    data.push_back(1.0f);
                }
            }
        }
        return data;
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