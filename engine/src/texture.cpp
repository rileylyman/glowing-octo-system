#include <stdlib.h>
#include <iostream>
#include <stb_image.h>
#include "engine/texture.h"

GLenum glCheckError_(const char *file, int line)
{
        GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}

Cubemap::Cubemap(std::vector<std::string> filenames, uint32_t unit, bool srgb) {
    this->unit = unit;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(false);
    int width, height, nrChannels;
    unsigned char *data;  
    for(GLuint i = 0; i < filenames.size(); i++) {
        data = stbi_load(filenames[i].c_str(), &width, &height, &nrChannels, 0);
        if (!data) {
            std::cout << "Could not load cubemap texture file " << i << std::endl;
            exit(EXIT_FAILURE);
        }
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
            0, srgb ? GL_SRGB : GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
        );
        stbi_image_free(data);
    }
}

Texture::Texture(std::string filename, uint32_t unit, bool srgb) : unit(unit) {

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = GL_RED;
        if (nrChannels == 3) {
            format = GL_RGB;
        } else if (nrChannels == 4) {
            format = GL_RGBA;
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, srgb ? GL_SRGB : GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
        exit(EXIT_FAILURE);
    }
    stbi_image_free(data);
}

void Texture::use() {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}