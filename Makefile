LIBS=`pkg-config --libs gl glew glfw3`
TARGET=particles

all:
	g++ main.cpp shaders.cpp -o ${TARGET} ${LIBS}
