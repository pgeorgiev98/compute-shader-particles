#ifndef SHADERS_H_INCLUDED
#define SHADERS_H_INCLUDED

#include <GL/glew.h>
#include <GL/gl.h>

#include <string>
#include <vector>
#include "config.h"

GLuint createShader(const std::string &fileName, GLenum shaderType, Config& config, const std::string &shaderName = "");
GLuint linkProgram(const std::vector<GLuint> &shaders);

#endif
