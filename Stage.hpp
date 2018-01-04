#include "TSOHeap.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
using namespace std;;

struct IStage{
    virtual void run() = 0; 
    virtual void wait_end() = 0;
    virtual void stage_func() = 0;
    virtual double get_exec_time() = 0;
    virtual void reset_exec_time()=0; 
    virtual void add_next(IStage&)=0;   
    virtual IStage* get_next() = 0;
    virtual void* get_input_ptr() = 0; 
    virtual void set_input(void*) = 0;
    virtual void collapse() = 0;
    virtual bool is_collapsed() = 0;
    virtual void collapse_next_stage() = 0;
    virtual int num_collapsed() = 0;
    ~IStage(){};    
};

template <typename Tin, typename Tf, typename Tout>
struct Stage : IStage{

    Stage(Tf  function, int ind):fun{function}, input_ptr{new(TSOHeap<Tin>)},_end{false},
	     next{nullptr}, collapsed{0}, i{ind}, exec_time{0.0},count{0},collapsing{false}{};	
   
    ~Stage(){delete input_ptr;}

    void stage_func(){ 
	pair<Tin*,int> input_pair = input_ptr->pop();
	Tin * input = input_pair.first;
	int id = input_pair.second;
	if (input!=nullptr){
            auto start = chrono::system_clock::now();   
	    Tout out = fun(*input);  
            auto end = chrono::system_clock::now();
	    chrono::duration<double> diff = end-start;
	    exec_time = diff.count(); 
	    if (next!=nullptr)
		next->set_input(new Tout(out));           
	}
	else
	    _end = true;   
    }

    void run_thread(){
        while(!_end){
	    while(collapsing); //waiting that next stage finishes the remaining tasks
	    stage_func();
	    if(collapsed==1 && !_end)	      
		next->stage_func();			                
    	} 
        if(collapsed!=-1){ 	
	    IStage * nptr = next;
	    if(nptr!=nullptr && nptr->is_collapsed())	  
	        nptr = nptr->get_next();	    
	    if(nptr!=nullptr)
		nptr->set_input(nullptr);
	}
	else{
	    while((input_ptr->size)>0)
		stage_func();
	}
    }

    void run(){ 
	thread _t(&Stage::run_thread, this);
	t = move(_t);
        return;
    }

    void wait_end(){
	t.join();
    }

    void set_input(void * iptr){
        input_ptr->push(static_cast<Tin*>(iptr), ++count);
     }

    void* get_input_ptr(){
        return input_ptr;
    }

    void add_next(IStage &n){
        next = &n;
	output_ptr = static_cast<TSOHeap<Tout>*>(n.get_input_ptr());      
    }

    void collapse(){       
        collapsed=-1;
        input_ptr->push(nullptr, INT_MIN);
//TODO prima condizione è per evitare deadlock se questo thread conclude esecuzione nel frattempo
	while(!_end && (input_ptr->size)>0);      
    }

    bool is_collapsed(){
        return collapsed==-1;
    }

    void collapse_next_stage(){ 
	collapsing = true;	
	next->collapse();
	collapsed++;
	collapsing = false; 
	cout << "Stage # " << i << " has collapsed the successive Stage" << endl;
    }

    IStage* get_next(){
        return next;
    }

    double get_exec_time(){
        return exec_time;
    }

    void reset_exec_time(){
        exec_time = 0;
    }

    int num_collapsed(){
        return collapsed;
    }

    Tf fun;
    TSOHeap<Tout> * output_ptr;
    TSOHeap<Tin> * input_ptr;
    thread t;  
    IStage * next;
    bool _end;
    int collapsed;
    int const i; //for debug
    double exec_time;
    int count;
    bool collapsing;
};

