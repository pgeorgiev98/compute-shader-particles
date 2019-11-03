#ifndef SHADERS_H_INCLUDED
#define SHADERS_H_INCLUDED

#include <GL/glew.h>
#include <GL/gl.h>

#include <string>
#include <vector>

GLuint createShader(const std::string &code, GLenum shaderType, const std::string &shaderName = "");
GLuint linkProgram(const std::vector<GLuint> &shaders);

#endif
