LIBS=`pkg-config --libs gl glew glfw3`
TARGET=particles

all:
	g++ main.cpp -o ${TARGET} ${LIBS}
