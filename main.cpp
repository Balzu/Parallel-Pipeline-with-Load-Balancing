#include<iostream>
#include<functional>
#include <chrono>
#include "Pipe.hpp"
using namespace std;;

int main(int argc, char* argv[]){
    auto f = [] (int x) {return x*x;};
    Stage<int,function<int(int)>,int> s{
        [](int x){return x*x;}};

    s.set_input(4);
    s.run();
    this_thread::sleep_for(chrono::milliseconds(1000));
    s.set_input_ptr(nullptr);
    this_thread::sleep_for(chrono::milliseconds(1000));
    s.end_s();
    s.wait_end();    
    cout << "Output of stage thread: " << s.get_output() << endl;
/*
    Stage<int,function<int(int)>,int> s2{f};
    
    Pipe<int,int> p (&s, &s2);
    cout << "Pipe length: " << p.num_nodes() << endl;
    Stage<int,function<float(int)>,float> s3 {
        [](int x) {return ((float)(x*x/2.24));}
    };

    Pipe<int,float> p2 (&p, &s3);

    cout << "Pipe 2 length: " << p2.num_nodes() << endl;
    p2.set_input(4);
    p2.run();
    float* optr =  static_cast<float*>(p2.get_output_ptr());
    cout << "Final result P2 " << *optr << endl;//TODO
*/
    return 0;
}

