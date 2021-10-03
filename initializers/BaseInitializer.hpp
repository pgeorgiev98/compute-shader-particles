#ifndef BASEINITIALIZER_HPP
#define BASEINITIALIZER_HPP

#include <GL/glew.h>
#include "../config.h"

struct Textures {
    GLuint mainTextureID;
    GLuint particleCountTextureID;
    GLuint particlePositionTextureID;
    GLuint particleDestinationTextureID;
    GLuint particleMassTextureID;
    GLuint colorTextureRID;
    GLuint colorTextureGID;
    GLuint colorTextureBID;
    GLuint imageTextureID;
    int imageWidth;
    int imageHeight;
};

class BaseInitializer {
public:
    Textures initializeTextures(const Config &config) const;
    void loadImage(const char *imagePath, Textures &textures) const;

protected:

    virtual GLuint generateMainTexture(const Config &config) const;
    virtual GLuint generateParticlePositionTexture(const Config &config) const;
    virtual GLuint generateParticleCountTexture(const Config &config) const;
    virtual GLuint generateParticleDestinationsTexture(const Config &config) const;
    virtual GLuint generateParticleMassesTexture(const Config &config) const;

    void getnerateOutputColorTextures(const Config &config, Textures &textures) const;
//    void loadImage(const char *imagePath, Textures &textures) const;
};


#endif //BASEINITIALIZER_HPP
