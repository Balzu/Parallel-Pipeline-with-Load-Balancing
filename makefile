all: TSOHeap.hpp Stage.hpp Pipe.hpp
	g++ main.cpp -std=c++11 -pthread -g -O0 
