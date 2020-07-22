#include "shaders.h"
#include "config.h"

#include <GL/glew.h>
#include <GL/gl.h>

#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <streambuf>

using namespace std;

static string formatShaderCode(Config& config, string shaderCode) {
    for (auto v : config.values) {
        string name = v.first;
        string s = "${" + name + "}";
        for (;;) {
            auto i = shaderCode.find(s);
            if (i == string::npos)
                break;
            shaderCode.replace(i, s.size(), config.getValue(name));
        }
    }
    return shaderCode;
}


GLuint createShader(const string &fileName, GLenum shaderType, Config& config, const string &shaderName)
{
    std::ifstream shaderFile(fileName);
    std::string code((std::istreambuf_iterator<char>(shaderFile)),
                     std::istreambuf_iterator<char>());

    code = formatShaderCode(config, code);

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
		cerr << "Failed to compile shader \"" << shaderName << "\": " << log << endl;
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
