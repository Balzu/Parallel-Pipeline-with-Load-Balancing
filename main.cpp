#include<iostream>
#include<functional>
#include <chrono>
#include "Pipe.hpp"
using namespace std;;

int main(int argc, char* argv[]){
    auto f = [] (int x) {return x+x;};
    Stage<int,function<int(int)>,int> s{
        [](int x){
	    for(int i=0; i<1000; i++)
                i=i;
	    return x+x;
	},1};
    Stage<int,function<int(int)>,int> s2{f,2};
    Stage<int,function<int(int)>,int> s3{f,3};
    Stage<int,function<int(int)>,int> s4{f,4};
   

   /* LB Pipeline by hand 
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

    s.collapse_next_stage();

    s.set_input(2);
    this_thread::sleep_for(chrono::milliseconds(1000));
    cout << "Output of last stage : " << s4.get_output() << endl;
    
    s.set_input_ptr(nullptr);
    this_thread::sleep_for(chrono::milliseconds(1000));

    s.wait_end();   
    s2.wait_end();   
    s3.wait_end();   
    s4.wait_end();   
*/

    
    Pipe<int,int> p (&s, &s2, &s3, &s4);
    cout << "Pipe length: " << p.num_nodes() << endl;
    p.run({1,2,3});











    return 0;
}

