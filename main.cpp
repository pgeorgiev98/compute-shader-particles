#include <iostream>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#define WINDOW_TITLE "Particles"

static GLFWwindow *window = nullptr;

static const int width = 512, height = 512;
static const bool enableVSync = true;

int main()
{
	if (!glfwInit()) {
		cerr << "Failed to initialize GLFW" << endl;
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(width, height, WINDOW_TITLE, nullptr, nullptr);
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

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glfwSwapInterval(enableVSync);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}
