#include "config.h"
#include "shaders.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <variant>
using namespace std;
# define M_PI           3.14159265358979323846

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

static GLFWwindow *window = nullptr;
static int frames = 0;

static Config config;

struct Textures {
    GLuint mainTextureID;
    GLuint particleCountTextureID;
    GLuint particlePositionTextureID;
    GLuint particleDestinationTextureID;
    GLuint particleMassTextureID;
    GLuint imageTextureID;
    GLuint colorTextureRID;
    GLuint colorTextureGID;
    GLuint colorTextureBID;
};

static void printHelp(const char *arg0);
static void printConfig();
static void handleArguments(int argc, char **argv);
static void initializeGLFW();
void initializeTextures(Textures& textures);
void generateViewPane(GLuint renderProgramID);

void draw(const Textures &textures, GLuint renderProgramID, GLuint renderTexureProgramID, GLuint updateParticlesProgramID);

int main(int argc, char **argv)
{
    handleArguments(argc, argv);
    initializeGLFW();

    Textures textures{};
    initializeTextures(textures);


	// Create the render program
	GLuint vertexID = createShader("vertex.glsl", GL_VERTEX_SHADER, config, "Vertex shader");
	GLuint fragmentID = createShader("fragment.glsl", GL_FRAGMENT_SHADER, config, "Fragment shader");
	GLuint renderProgramID = linkProgram({vertexID, fragmentID});

	// Create the shader programs
	GLuint renderTexureProgramID = linkProgram({createShader("renderTextureCompute.glsl", GL_COMPUTE_SHADER, config, "Render texture compute shader")});
	GLuint updateParticlesProgramID = linkProgram({createShader("updateParticlesCompute.glsl", GL_COMPUTE_SHADER, config, "Update particles compute shader")});

    generateViewPane(renderProgramID);

    // Drawing
    draw(textures, renderProgramID, renderTexureProgramID, updateParticlesProgramID);

    return 0;
}

void draw(const Textures &textures, GLuint renderProgramID, GLuint renderTexureProgramID, GLuint updateParticlesProgramID) {
    glClearColor(0.0, 0.0, 0.0, 1.0);

    glfwSwapInterval(config.enableVSync);
    glfwSetTime(0.0);
    double lastFrameTime = 0.0;
    double fpsCountLastTime = 0.0;
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double timeSinceLastFrame = currentTime - lastFrameTime;
        lastFrameTime += timeSinceLastFrame;

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
        glBindImageTexture(0, textures.mainTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindImageTexture(1, textures.particleCountTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindImageTexture(2, textures.particlePositionTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        glActiveTexture(GL_TEXTURE0 + 3);
        glBindImageTexture(3, textures.particleMassTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
        glActiveTexture(GL_TEXTURE0 + 4);
        glBindImageTexture(4, textures.particleDestinationTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glActiveTexture(GL_TEXTURE0 + 5);
        glBindImageTexture(5, textures.colorTextureRID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
        glActiveTexture(GL_TEXTURE0 + 6);
        glBindImageTexture(6, textures.colorTextureGID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
        glActiveTexture(GL_TEXTURE0 + 7);
        glBindImageTexture(7, textures.colorTextureBID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
        glActiveTexture(GL_TEXTURE0 + 8);
        glBindImageTexture(8, textures.imageTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        // Update the particles
        glUseProgram(updateParticlesProgramID);
        glUniform1i(glGetUniformLocation(updateParticlesProgramID, "particleCountTexture"), 1);
        glUniform1i(glGetUniformLocation(updateParticlesProgramID, "particlePositionTexture"), 2);
        glUniform1i(glGetUniformLocation(updateParticlesProgramID, "particleMassTexture"), 3);
        glUniform1i(glGetUniformLocation(updateParticlesProgramID, "particleDestinationTexture"), 4);
        glUniform1i(glGetUniformLocation(updateParticlesProgramID, "colorTextureR"), 5);
        glUniform1i(glGetUniformLocation(updateParticlesProgramID, "colorTextureG"), 6);
        glUniform1i(glGetUniformLocation(updateParticlesProgramID, "colorTextureB"), 7);
        glUniform1i(glGetUniformLocation(updateParticlesProgramID, "imageTexture"), 8);
        glUniform2f(glGetUniformLocation(updateParticlesProgramID, "cursorPos"), xpos, ypos);
        glUniform1f(glGetUniformLocation(updateParticlesProgramID, "timeSinceLastFrame"), timeSinceLastFrame);
        glUniform1f(glGetUniformLocation(updateParticlesProgramID, "forceMultiplier"), forceMultiplier);
        glDispatchCompute(config.particleCountX / 16, config.particleCountY / 16, 1);

        // Render the output texture
        glUseProgram(renderTexureProgramID);
        glUniform1i(glGetUniformLocation(updateParticlesProgramID, "outputTexture"), 0);
        glUniform1i(glGetUniformLocation(renderTexureProgramID, "particleCountTexture"), 1);
        glUniform1i(glGetUniformLocation(renderTexureProgramID, "colorTextureR"), 5);
        glUniform1i(glGetUniformLocation(renderTexureProgramID, "colorTextureG"), 6);
        glUniform1i(glGetUniformLocation(renderTexureProgramID, "colorTextureB"), 7);
        glDispatchCompute(config.width / 16, config.height / 16, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glUseProgram(renderProgramID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures.mainTextureID);
        glUniform1i(glGetUniformLocation(renderProgramID, "sampler"), 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();

        ++frames;
        double timeSinceLastFpsCount = currentTime - fpsCountLastTime;
        if (timeSinceLastFpsCount >= 1.0) {
            cerr << "FPS: " << frames / timeSinceLastFpsCount << endl;
            frames = 0;
            fpsCountLastTime = currentTime;
        }
    }
}

void generateViewPane(GLuint renderProgramID) {// Two triangles
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, data, GL_STREAM_DRAW);
    GLint posPtr = glGetAttribLocation(renderProgramID, "pos");
    glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posPtr);
}

void initializeTextures(Textures &textures) {
    // Create the main texture
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

        glGenTextures(1, &textures.mainTextureID);
        glBindTexture(GL_TEXTURE_2D, textures.mainTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, config.width, config.height, 0, GL_RGBA, GL_FLOAT, data);
        delete[] data;
    }

    // Create a integer texture, where every pixel's value is the number of particles
    // that should be positioned on the corresponding pixel on the main texture/screen
    {
        auto sz = config.width * config.height * 4;
        GLuint *data = new GLuint[sz];
        for (int i = 0; i < sz; ++i)
            data[i] = 2;
        glGenTextures(1, &textures.particleCountTextureID);
        glBindTexture(GL_TEXTURE_2D, textures.particleCountTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, config.width, config.height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
        delete[] data;
    }

    // Create a texture in which the particle coordinates will be stored
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

        glGenTextures(1, &textures.particlePositionTextureID);
        glBindTexture(GL_TEXTURE_2D, textures.particlePositionTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeX, sizeY, 0, GL_RGBA, GL_FLOAT, data);
        delete[] data;
    }

    // Create a texture in which the particle destinations are
    {
        int sizeX = config.particleCountX;
        int sizeY = config.particleCountY;

        auto sz = sizeX * sizeY;
        float *data = new float[sz * 4];
        for (int i = 0; i < sz; ++i) {
            float positionX = (rand() / float(RAND_MAX)) * config.width;
            float positionY = (rand() / float(RAND_MAX)) * config.height;
//            float radius = sqrt(rand() / float(RAND_MAX));
//            data[i * 4 + 0] = 150 + 50.0f * cos(angle) * radius;
//            data[i * 4 + 1] = 150 + 50.0f * sin(angle) * radius;
            data[i * 4 + 0] = positionX;
            data[i * 4 + 1] = positionY;
            data[i * 4 + 2] = 0.0;
            data[i * 4 + 3] = 0.0;
        }

        glGenTextures(1, &textures.particleDestinationTextureID);
        glBindTexture(GL_TEXTURE_2D, textures.particleDestinationTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeX, sizeY, 0, GL_RGBA, GL_FLOAT, data);
        delete[] data;
    }

    // Create a texture in which the particle masses will be stored
    {
        int sizeX = config.particleCountX;
        int sizeY = config.particleCountY;

        auto sz = sizeX * sizeY;
        float *data = new float[sz];
        for (int i = 0; i < sz; ++i) {
            data[i] = config.massMin + (config.massMax - config.massMin) * (float(rand()) / RAND_MAX);
        }

        glGenTextures(1, &textures.particleMassTextureID);
        glBindTexture(GL_TEXTURE_2D, textures.particleMassTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, sizeX, sizeY, 0, GL_RED, GL_FLOAT, data);
        delete[] data;
    }

    // Create a texture in which the output color will be stored
    {
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

    {
        const char* imagePath = "images/icaka.jpg";
        int width, height, nrChannels;
        unsigned char *data = stbi_load(imagePath, &width, &height, &nrChannels, 0);
        if (!data) {
            cerr << "Failed loading image: " << imagePath << endl;
        }
        glGenTextures(1, &textures.imageTextureID);
        glBindTexture(GL_TEXTURE_2D, textures.imageTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
}

void printHelp(const char *arg0) {
    cout << "Usage: " << arg0 << " [OPTIONS]" << endl << endl
         << "OPTIONS:" << endl
         << "\thelp, --help, -h    Display this message" << endl
         << "\tprintConfig         Print all configurable values" << endl
         << "\tset name value      Set the value of {name} to {value}" << endl;
}

void printConfig() {
    int maxWidth = 0;
    for (auto v : config.values)
        if (v.first.size() > maxWidth)
            maxWidth = v.first.size();

    for (auto v : config.values) {
        string s = v.first;
        cout << setw(maxWidth) << s << " " << config.getValue(s) << endl;
    }
}

void handleArguments(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a == "help" || a == "--help" || a == "-h") {
            printHelp(argv[0]);
            exit(0);
        } else if (a == "printConfig") {
            printConfig();
            exit(0);
        } else if (a == "set") {
            if (i + 2 >= argc)
                throw "Expected 2 arguments to 'set'";
            config.setValue(argv[i + 1], argv[i + 2]);
            i += 2;
        } else {
            cerr << "Unknown argument: " << a << endl;
            exit(1);
        }
    }
}

void initializeGLFW() {
    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW" << endl;
        exit(1);
    }

    // Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(config.width, config.height, WINDOW_TITLE, nullptr, nullptr);
    if (window == nullptr) {
        cerr << "Failed to open window" << endl;
        exit(1);
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cerr << "Failed to initialize GLEW" << endl;
        exit(1);
    }

}
