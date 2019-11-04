LIBS=`pkg-config --libs gl glew glfw3`
TARGET=particles

all:
	g++ -std=c++1z main.cpp shaders.cpp -o ${TARGET} ${LIBS}
