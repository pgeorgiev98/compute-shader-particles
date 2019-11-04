#include "config.h"
#include "shaders.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <variant>
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

static string renderTextureComputeShaderCode = "\
#version 430\n\
layout(binding = 0, rgba32f) uniform writeonly image2D outputTexture;\n\
layout(binding = 1, r32ui) uniform uimage2D particleCountTexture;\n\
layout(local_size_x = 16, local_size_y = 16) in;\n\
float w = ${width}, h = ${height};\n\
void main() {\n\
	ivec2 pos = ivec2(gl_GlobalInvocationID.xy);\n\
	float v = imageAtomicExchange(particleCountTexture, pos, 0);\n\
	imageStore(outputTexture, pos, vec4(v * ${colorRedMul} + ${colorRedAdd}, v * ${colorGreenMul} + ${colorGreenAdd}, v * ${colorBlueMul} + ${colorBlueAdd}, 1.0));\n\
}";

static string updateParticlesComputeShaderCode = "\
#version 430\n\
layout(binding = 1, r32ui) uniform coherent uimage2D particleCountTexture;\n\
layout(binding = 2, rgba32f) uniform image2D particlePositionTexture;\n\
layout(binding = 3, r32f) uniform image2D particleMassTexture;\n\
layout(local_size_x = 16, local_size_y = 16) in;\n\
uniform float forceMultiplier;\
uniform vec2 cursorPos;\
float w = ${width}, h = ${height};\n\
void main() {\n\
	ivec2 id = ivec2(gl_GlobalInvocationID.xy);\n\
	vec4 v = imageLoad(particlePositionTexture, id);\n\
	float mass = imageLoad(particleMassTexture, id).r;\n\
	\
	float dx = v.x - cursorPos.x;\n\
	float dy = v.y - cursorPos.y;\n\
	float dist = dx * dx + dy * dy;\n\
	if (dist < ${minimumDistance}) dist = ${minimumDistance};\n\
	float c = forceMultiplier / mass;\n\
	v.z += c * dx / dist;\n\
	v.w += c * dy / dist;\n\
	v.x -= v.z;\n\
	v.y -= v.w;\n\
	\
	//if (v.x < 0.0) { v.x = 0.0; v.z = 0.0; }\n\
	//if (v.x > ${width}) { v.x = ${width}; v.z = 0.0; }\n\
	//if (v.y < 0.0) { v.y = 0.0; v.w = 0.0; }\n\
	//if (v.y > ${height}) { v.y = ${height}; v.w = 0.0; }\n\
	\
	if (v.x < 0.0) { v.x = 0.0; v.z *= -0.5; v.w *= 0.5; }\n\
	if (v.x > ${width}) { v.x = ${width}; v.z *= -0.5; v.w *= 0.5; }\n\
	if (v.y < 0.0) { v.y = 0.0; v.w *= -0.5; v.z *= 0.5; }\n\
	if (v.y > ${height}) { v.y = ${height}; v.w *= -0.5; v.z *= 0.5; }\n\
	\
	imageStore(particlePositionTexture, id, v);\n\
	imageAtomicAdd(particleCountTexture, ivec2(v.x, v.y), 1);\n\
}";

string formatShaderCode(string shaderCode)
{
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


static void printHelp(const char *arg0)
{
	cout << "Usage: " << arg0 << " [OPTIONS]" << endl << endl
		 << "OPTIONS:" << endl
		 << "\thelp, --help, -h    Display this message" << endl
		 << "\tprintConfig         Print all configurable values" << endl
		 << "\tset name value      Set the value of {name} to {value}" << endl;
}

static void printConfig()
{
	for (auto v : config.values) {
		string s = v.first;
		cout << setw(20) << s << " " << config.getValue(s) << endl;
	}
}

int main(int argc, char **argv)
{
	for (int i = 1; i < argc; ++i) {
		string a = argv[i];
		if (a == "help" || a == "--help" || a == "-h") {
			printHelp(argv[0]);
			return 0;
		} else if (a == "printConfig") {
			printConfig();
			return 0;
		} else if (a == "set") {
			if (i + 2 >= argc)
				throw "Expected 2 arguments to 'set'";
			config.setValue(argv[i + 1], argv[i + 2]);
			i += 2;
		} else {
			cerr << "Unknown argument: " << a << endl;
			return 1;
		}
	}

	// Initialize GLFW
	if (!glfwInit()) {
		cerr << "Failed to initialize GLFW" << endl;
		return 1;
	}

	// Create window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, false);
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
	}

	// Create a integer texture, where every pixel's value is the number of particles
	// that should be positioned on the corresponding pixel on the main texture/screen
	GLuint particleCountTextureID;
	{
		auto sz = config.width * config.height * 4;
		GLuint *data = new GLuint[sz];
		for (int i = 0; i < sz; ++i)
			data[i] = 2;
		glGenTextures(1, &particleCountTextureID);
		glBindTexture(GL_TEXTURE_2D, particleCountTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, config.width, config.height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
		delete[] data;
	}

	// Create a texture in which the particle coordinates will be stored
	GLuint particlePositionTextureID;
	{
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

		glGenTextures(1, &particlePositionTextureID);
		glBindTexture(GL_TEXTURE_2D, particlePositionTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeX, sizeY, 0, GL_RGBA, GL_FLOAT, data);
		delete[] data;
	}

	// Create a texture in which the particle masses will be stored
	GLuint particleMassTextureID;
	{
		int sizeX = config.particleCountX;
		int sizeY = config.particleCountY;

		auto sz = sizeX * sizeY;
		float *data = new float[sz];
		for (int i = 0; i < sz; ++i) {
			data[i] = config.massMin + (config.massMax - config.massMin) * (float(rand()) / RAND_MAX);
		}

		glGenTextures(1, &particleMassTextureID);
		glBindTexture(GL_TEXTURE_2D, particleMassTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, sizeX, sizeY, 0, GL_RED, GL_FLOAT, data);
		delete[] data;
	}

	// Create the render program
	GLuint vertexID = createShader(vertexShaderCode, GL_VERTEX_SHADER, "Vertex shader");
	GLuint fragmentID = createShader(fragmentShaderCode, GL_FRAGMENT_SHADER, "Fragment shader");
	GLuint renderProgramID = linkProgram({vertexID, fragmentID});

	// Create the shader programs
	GLuint renderTexureProgramID = linkProgram({createShader(formatShaderCode(renderTextureComputeShaderCode), GL_COMPUTE_SHADER, "Render texture compute shader")});
	GLuint updateParticlesProgramID = linkProgram({createShader(formatShaderCode(updateParticlesComputeShaderCode), GL_COMPUTE_SHADER, "Update particles compute shader")});

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

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		ypos = config.height - ypos;

		float forceMultiplier = 0.0;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			forceMultiplier = -1.0;
		else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			forceMultiplier = 1.0;
		forceMultiplier *= config.forceMultiplier;

		glActiveTexture(GL_TEXTURE0);
		glBindImageTexture(0, textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindImageTexture(1, particleCountTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindImageTexture(2, particlePositionTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindImageTexture(3, particleMassTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

		// Update the particles
		glUseProgram(updateParticlesProgramID);
		glUniform1i(glGetUniformLocation(updateParticlesProgramID, "particleCountTexture"), 1);
		glUniform1i(glGetUniformLocation(updateParticlesProgramID, "particlePositionTexture"), 2);
		glUniform1i(glGetUniformLocation(updateParticlesProgramID, "particleMassTexture"), 3);
		glUniform2f(glGetUniformLocation(updateParticlesProgramID, "cursorPos"), xpos, ypos);
		glUniform1f(glGetUniformLocation(updateParticlesProgramID, "forceMultiplier"), forceMultiplier);
		glDispatchCompute(config.particleCountX / 16, config.particleCountY / 16, 1);

		// Render the output texture
		glUseProgram(renderTexureProgramID);
		glUniform1i(glGetUniformLocation(updateParticlesProgramID, "outputTexture"), 0);
		glUniform1i(glGetUniformLocation(renderTexureProgramID, "particleCountTexture"), 1);
		glDispatchCompute(config.width / 16, config.height / 16, 1);

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
