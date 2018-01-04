#include<iostream>
#include<functional>
#include <chrono>
#include<cmath>
#include "Pipe.hpp"
using namespace std;;

auto iffun = [] (int x) {return (2.2*((float)x));};

auto fpfun = [] (float x) {return pair<string, float>("one",x);};

auto pprint = [] (pair<string,float>p) {
    cout << "first " << p.first << ", second " << p.second << endl;
    return p.second;
};

int main(int argc, char* argv[])
{
   
    Stage<int,function<float(int)>,float> s1{iffun,1};
    Stage<float,function<pair<string,float>(float)>,pair<string,float>> s2{fpfun,2};
    Stage<pair<string,float>,function<float(pair<string,float>)>,float> s3{pprint,3}; 

    Pipe<int,int> p ({&s1, &s2, &s3});
    cout << "Pipe length: " << p.num_nodes() << endl;
    list<int> li {};
    for(int i=0; i<100; i++)
	li.push_back(i);
    p.run(move(li));
    return 0;
}

