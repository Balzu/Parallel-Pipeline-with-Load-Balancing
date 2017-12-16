all: Node.hpp Stage.hpp Pipe.hpp
	g++ main.cpp -std=c++11 -pthread -g
