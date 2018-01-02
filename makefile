host: TSOHeap.hpp Stage.hpp Pipe.hpp
	icc main.cpp -std=c++11 -pthread -O0 -o hpipe -g

mic: TSOHeap.hpp Stage.hpp Pipe.hpp
	icc main.cpp -std=c++11 -mmic -pthread -O0 -o mpipe -g

gnu: TSOHeap.hpp Stage.hpp Pipe.hpp
	g++ main.cpp -std=c++11 -pthread -g 
