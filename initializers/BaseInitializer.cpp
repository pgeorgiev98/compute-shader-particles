#include "BaseInitializer.hpp"
#include "../stb_image.h"
#include <iostream>

Textures BaseInitializer::initializeTextures(const Config &config) const {
    Textures textures;
    textures.mainTextureID = generateMainTexture(config);

    // Create a integer texture, where every pixel's value is the number of particles
    // that should be positioned on the corresponding pixel on the main texture/screen
    textures.particleCountTextureID = generateParticleCountTexture(config);

    // Create a texture in which the particle coordinates will be stored
    textures.particlePositionTextureID = generateParticlePositionTexture(config);

    // Create a texture in which the particle destinations are
    textures.particleDestinationTextureID = generateParticleDestinationsTexture(config);

    // Create a texture in which the particle masses will be stored
    textures.particleMassTextureID = generateParticleMassesTexture(config);

    // Create textures in which the output color will be stored
    getnerateOutputColorTextures(config, textures);

    const char* imagePath = "images/icaka.png";
    loadImage(imagePath, textures);

    return textures;
}

void BaseInitializer::loadImage(const char *imagePath, Textures &textures) const {
    int nrChannels;
    unsigned char *data = stbi_load(imagePath, &textures.imageWidth, &textures.imageHeight, &nrChannels, 0);
    int sz = textures.imageWidth*textures.imageHeight*4;
    float *fdata = new float[sz];
    for (int i = 0; i < sz; ++i) {
        fdata[i] = data[i];
    }

    if (!data) {
        std::cerr << "Failed loading image: " << imagePath << std::endl;
    }

    glGenTextures(1, &textures.imageTextureID);
    glBindTexture(GL_TEXTURE_2D, textures.imageTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textures.imageWidth, textures.imageHeight, 0, GL_RGBA, GL_FLOAT, fdata);
    stbi_image_free(data);
}

void BaseInitializer::getnerateOutputColorTextures(const Config &config, Textures &textures) const {
    auto sz = config.width * config.height;
    GLuint  *data = new GLuint[sz];
    for (int i = 0; i < sz; ++i) {
        data[i] = 1;
    }

    glGenTextures(1, &textures.colorTextureRID);
    glBindTexture(GL_TEXTURE_2D, textures.colorTextureRID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, config.width, config.height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
    glGenTextures(1, &textures.colorTextureGID);
    glBindTexture(GL_TEXTURE_2D, textures.colorTextureGID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, config.width, config.height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
    glGenTextures(1, &textures.colorTextureBID);
    glBindTexture(GL_TEXTURE_2D, textures.colorTextureBID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, config.width, config.height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
    delete[] data;
}

GLuint BaseInitializer::generateParticleMassesTexture(const Config &config) const {
    GLuint particleMassTexture;
    int sizeX = config.particleCountX;
    int sizeY = config.particleCountY;

    auto sz = sizeX * sizeY;
    float *data = new float[sz];
    for (int i = 0; i < sz; ++i) {
        data[i] = config.massMin + (config.massMax - config.massMin) * (float(rand()) / RAND_MAX);
    }

    glGenTextures(1, &particleMassTexture);
    glBindTexture(GL_TEXTURE_2D, particleMassTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, sizeX, sizeY, 0, GL_RED, GL_FLOAT, data);
    delete[] data;
    return particleMassTexture;
}

GLuint BaseInitializer::generateParticleDestinationsTexture(const Config &config) const {
    GLuint particleDestinationTexture;
    int sizeX = config.particleCountX;
    int sizeY = config.particleCountY;

    auto sz = sizeX * sizeY;
    float *data = new float[sz * 4];
    for (int i = 0; i < sz; ++i) {
        float positionX = (rand() / float(RAND_MAX)) * config.width;
        float positionY = (rand() / float(RAND_MAX)) * config.height;
        data[i * 4 + 0] = positionX;
        data[i * 4 + 1] = positionY;
        data[i * 4 + 2] = 0.0;
        data[i * 4 + 3] = 0.0;
    }

    glGenTextures(1, &particleDestinationTexture);
    glBindTexture(GL_TEXTURE_2D, particleDestinationTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeX, sizeY, 0, GL_RGBA, GL_FLOAT, data);
    delete[] data;
    return particleDestinationTexture;
}

GLuint BaseInitializer::generateParticlePositionTexture(const Config &config) const {
    GLuint particlePositionTexture;
    int sizeX = config.particleCountX;
    int sizeY = config.particleCountY;

    auto sz = sizeX * sizeY;
    float *data = new float[sz * 4];
    for (int i = 0; i < sz; ++i) {
        data[i * 4 + 0] = config.width * (rand() / float(RAND_MAX));
        data[i * 4 + 1] = config.height * (rand() / float(RAND_MAX));
        data[i * 4 + 2] = 0.0;
        data[i * 4 + 3] = 0.0;
    }

    glGenTextures(1, &particlePositionTexture);
    glBindTexture(GL_TEXTURE_2D, particlePositionTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeX, sizeY, 0, GL_RGBA, GL_FLOAT, data);
    delete[] data;
    return particlePositionTexture;
}

GLuint BaseInitializer::generateMainTexture(const Config &config) const {// Create the main texture
    GLuint mainTexture;
    auto sz = config.width * config.height;
    float *data = new float[sz * 4];
    for (int i = 0; i < sz; ++i) {
        float c = float(i) / sz;
        data[i * 4 + 0] = c;
        data[i * 4 + 1] = c;
        data[i * 4 + 2] = c;
        data[i * 4 + 3] = 1.0;
    }

    glGenTextures(1, &mainTexture);
    glBindTexture(GL_TEXTURE_2D, mainTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, config.width, config.height, 0, GL_RGBA, GL_FLOAT, data);
    delete[] data;
    return mainTexture;
}

GLuint BaseInitializer::generateParticleCountTexture(const Config &config) const {
    GLuint particleCountTexture;
    auto sz = config.width * config.height * 4;
    GLuint *data = new GLuint[sz];
    for (int i = 0; i < sz; ++i)
        data[i] = 2;
    glGenTextures(1, &particleCountTexture);
    glBindTexture(GL_TEXTURE_2D, particleCountTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, config.width, config.height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
    delete[] data;
    return particleCountTexture;
}