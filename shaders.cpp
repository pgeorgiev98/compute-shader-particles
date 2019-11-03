#include "shaders.h"

#include <GL/glew.h>
#include <GL/gl.h>

#include <iostream>
#include <stdlib.h>

using namespace std;

GLuint createShader(const string &code, GLenum shaderType, const string &shaderName)
{
	GLuint handle = glCreateShader(shaderType);
	const char *codePtr = code.c_str();
	glShaderSource(handle, 1, &codePtr, nullptr);
	glCompileShader(handle);

	GLint compileStatus;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &compileStatus);
	if (!compileStatus) {
		GLint logLength;
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);
		string log;
		log.resize(logLength);
		char *logPtr = &log[0];
		glGetShaderInfoLog(handle, logLength, nullptr, logPtr);
		cerr << "Failed to compile shader" << shaderName << ": " << log << endl;
		exit(1);
	}

	return handle;
}

GLuint linkProgram(const vector<GLuint> &shaders)
{
	GLuint handle = glCreateProgram();
	for (GLuint shader : shaders)
		glAttachShader(handle, shader);
	
	glLinkProgram(handle);
    GLint linkStatus;
    glGetProgramiv(handle, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus) {
    	cerr << "Failed to link program" << endl;
    	exit(1);
	}

	return handle;
}
