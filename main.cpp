#include<iostream>
#include<functional>
#include <chrono>
#include<cmath>
#include "Pipe.hpp"
using namespace std;;

auto f = [](int x){
    int c = 0;
    for(int i=0; i<300; i++)
        c=sin(i);
    return x;    
};

auto fast = [] (int x) {return x;};

auto fast_init = [](int x){
    if(x < 5)
        return x;
    
    int c=0;
    for(int i=0; i<300; i++)
        c=sin(i);    
    return x;    
};

auto print = [] (int x) {
    cout << "Result: " << x << " " << endl; 
    return ((float)x); 
};

int main(int argc, char* argv[])
{
    auto print_usage_msg = [&](){
    cout << "Usage: " << argv[0] << " <func_type> \n" <<
    "<func_type> = \n"
    " 0      to have 2 consecutive stages running the fast function\n"
    " 1      to have 2 consecutive stages running the fast function "
    "but after a short time reaching steady state " << endl;
    };

    if(argc!=2){
	print_usage_msg();
	return 1;
    }   

    int fun_code = atoi(argv[1]);

    if (fun_code!=0 && fun_code!=1){
        print_usage_msg();
	return 1;
    }   

    Stage<int,function<int(int)>,int> s1{f,1};
    Stage<int,function<int(int)>,int> s2{f,2};
    Stage<int,function<int(int)>,int> s3{f,3}; 
    Stage<int,function<int(int)>,int> s4{f,4};
    Stage<int,function<int(int)>,int> s5{f,5};
    Stage<int,function<int(int)>,int> s6{f,6};
    Stage<int,function<int(int)>,int> s7{f,7};
    Stage<int,function<float(int)>,float> sp{print,8};
    
    if(fun_code==0){
  	s2.fun = fast;
	s3.fun = fast; 
    }
    else{
  	s2.fun = fast_init;
	s3.fun = fast_init;
    }

    Pipe<int,float> p ({&s1, &s2, &s3, &s4, &s5, &s6, &s7, &sp});
    cout << "Pipe length: " << p.num_nodes() << endl;
    list<int> li {};
    for(int i=0; i<100; i++)
	li.push_back(i);
    p.run(move(li));
    return 0;
}

