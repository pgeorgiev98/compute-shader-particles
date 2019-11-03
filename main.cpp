#include "config.h"
#include "shaders.h"

#include <iostream>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

static GLFWwindow *window = nullptr;
static int frames = 0;

static Config config;

static string vertexShaderCode = "\
#version 430\n\
out vec2 uv;\n\
in vec2 pos;\n\
void main() {\n\
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);\n\
	uv = 0.5 * pos + 0.5;\n\
}";

static string fragmentShaderCode = "\
#version 430\n\
in vec2 uv;\n\
uniform sampler2D sampler;\n\
out vec3 color;\n\
void main() {\n\
	color = texture(sampler, uv).rgb;\n\
}";

// TODO: Remove hardcoded width and height
static string computeShaderCode = "\
#version 430\n\
layout(binding = 0, rgba32f) uniform image2D outputTexture;\n\
layout(local_size_x = 16, local_size_y = 16) in;\n\
void main() {\n\
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);\n\
	float r = storePos.x / 512.0;\n\
	float g = storePos.y / 512.0;\n\
	float b = 0.0;\n\
	imageStore(outputTexture, storePos, vec4(r, g, b, 1.0));\n\
}";


static void checkForErrors(string where)
{
	GLenum e = glGetError();
	if (e != GL_NO_ERROR) {
		cerr << "OpenGL error in " << where << ": " <<  gluErrorString(e) << " (" << e << ")" << endl;
		exit(1);
	}
}

int main()
{
	// Initialize GLFW
	if (!glfwInit()) {
		cerr << "Failed to initialize GLFW" << endl;
		return 1;
	}

	// Create window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(config.width, config.height, WINDOW_TITLE, nullptr, nullptr);
	if (window == nullptr) {
		cerr << "Failed to open window" << endl;
		return 1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return 1;
	}


	// Create the main texture
	GLuint textureID;
	{
		auto sz = config.width * config.height;
		float *data = new float[sz * 4];
		for (int i = 0; i < sz; ++i) {
			float c = float(i) / sz;
			data[i * 4 + 0] = c;
			data[i * 4 + 1] = c;
			data[i * 4 + 2] = c;
			data[i * 4 + 3] = 1.0;
		}

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, config.width, config.height, 0, GL_RGBA, GL_FLOAT, data);
		delete[] data;
		checkForErrors("Generate texture");
	}

	// Create the render program
	GLuint vertexID = createShader(vertexShaderCode, GL_VERTEX_SHADER, "Vertex shader");
	GLuint fragmentID = createShader(fragmentShaderCode, GL_FRAGMENT_SHADER, "Fragment shader");
	GLuint renderProgramID = linkProgram({vertexID, fragmentID});

	// Create the shader program
	GLuint computeProgramID = linkProgram({createShader(computeShaderCode, GL_COMPUTE_SHADER, "Compute shader")});

	// Two triangles
	GLuint vertArray;
	glGenVertexArrays(1, &vertArray);
	glBindVertexArray(vertArray);

	GLuint posBuf;
	glGenBuffers(1, &posBuf);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	float data[] = {
		-1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, data, GL_STREAM_DRAW);
	GLint posPtr = glGetAttribLocation(renderProgramID, "pos");
	glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posPtr);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glfwSwapInterval(config.enableVSync);
	glfwSetTime(0.0);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(computeProgramID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glBindImageTexture(0, textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glUniform1i(glGetUniformLocation(computeProgramID, "outputTexture"), 0);
		glDispatchCompute(config.width / 16, config.height / 16, 1);
		checkForErrors("Compute");

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glUseProgram(renderProgramID);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(glGetUniformLocation(renderProgramID, "sampler"), 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glfwSwapBuffers(window);
		glfwPollEvents();

		++frames;
		double time = glfwGetTime();
		if (time >= 1.0) {
			glfwSetTime(0.0);
			cerr << "FPS: " << frames / time << endl;
			frames = 0;
		}
	}

	return 0;
}
