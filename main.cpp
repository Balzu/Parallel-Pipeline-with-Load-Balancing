#include<iostream>
#include<functional>
#include <chrono>
#include "Pipe.hpp"
using namespace std;;

int main(int argc, char* argv[]){
    auto f = [] (int x) {return x*x;};
    Stage<int,function<int(int)>,int> s{
        [](int x){return x*x;},1};
    Stage<int,function<int(int)>,int> s2{f,2};
    Stage<int,function<int(int)>,int> s3{f,3};
    Stage<int,function<int(int)>,int> s4{f,4};
    
    cout << "Building LB pipeline by hand\n";
    s.add_next(s2);
    s2.add_next(s3);
    s3.add_next(s4);
  
    s.run();
    s2.run();
    s3.run();
    s4.run();
    s.set_input(2);
    this_thread::sleep_for(chrono::milliseconds(1000));
    cout << "Output of last stage : " << s4.get_output() << endl;
   
 //   s.collapsed = 1; 
    s.collapse_next_stage();
//    s2.set_input_ptr(nullptr);
//    s.collapse_next_stage();
    s.set_input(2);
    this_thread::sleep_for(chrono::milliseconds(1000));
    cout << "Output of last stage : " << s4.get_output() << endl;
    
    s.set_input_ptr(nullptr);
    this_thread::sleep_for(chrono::milliseconds(1000));
   // s.end_s();
    s.wait_end();   
    s2.wait_end();   
    s3.wait_end();   
    s4.wait_end();   
     
  //  cout << "Output of stage thread: " << s.get_output() << endl;
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

