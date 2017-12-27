#include "Node.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
using namespace std;;

struct IStage : Node{
    virtual void stage_func() = 0;
    Node * next;
    virtual void collapse() = 0;
    virtual IStage* get_next() = 0;
    virtual bool is_collapsed() = 0;
    virtual double get_exec_time() = 0; 
    virtual void collapse_next_stage() = 0;
    virtual void wait_end() = 0;
    virtual int num_collapsed() = 0;
};

template <typename Tin, typename Tf, typename Tout>
struct Stage : IStage{

    Stage(Tf  function, int ind):fun{function}, input_ptr{new(Tin)},output_ptr{new(Tout)},
	     next{nullptr}, new_input{false}, collapsed{0}, i{ind}, exec_time{0.0}{};	
   
    void stage_func(){ 
	Tin input = *input_ptr;
        auto start = chrono::system_clock::now();
        Tout out = fun(*input_ptr);  // compute-intensive line..
        auto end = chrono::system_clock::now();
	chrono::duration<double> diff = end-start;
	exec_time = diff.count();
	if (next!=nullptr)  // wait for next node to be ready only if next node exists
	//se è collassato, inutile aspettare perchè tanto lo esegue questo thread
	while(!(next->is_ready()) && !(((IStage*)next)->is_collapsed())){}; 
	*output_ptr = out;
	if(next!=nullptr) next->set_new_input();
        new_input = false;        	
	return;
    }

    void run_thread(){
        while(!end()){ 
            while(!end() && !new_input){}; //spinning..    
            if(!end()){	
	        stage_func();
		if(collapsed!=0){ //this thread has to run more Stages
		    IStage * nptr = static_cast<IStage*> (next);
		    for(int i=0; i< collapsed; i++){
		        nptr->stage_func();
			nptr = nptr->get_next();
		    }
		}
            }
    	} //Finalization..       
        if(next!=nullptr && collapsed!=-1){ //collapsed = -1 means to stop only this thread (because of collapsing) TODO: puoi togliere prima guardia, perchè testi sotto
	// If the next stages are collapsed, then they are already ended. 
	// So, we just need to set nullptr as input to the next active thread (if any)
	    IStage * nptr = static_cast<IStage*>(next);		 
	    while(nptr!=nullptr && nptr->is_collapsed()){	  
	        nptr = nptr->get_next();		
	    }
	    if(nptr!=nullptr) nptr->set_input_ptr(nullptr);
	}
    }

    void run(){
        thread t(&Stage::run_thread, this);       
        threads.push_back(move(t));
        return;
    }

    void wait_end(){
        threads[0].join();
    }

    void run_and_wait_end(){  //TODO: ha senso solo su Pipeline?
        cout << "Running node and waiting end\n";
        run();
        wait_end();
    }

    // Non è l'oggetto a essere distrutto, ma il thread a essere terminato
    bool end(){
        return (input_ptr==nullptr || collapsed==-1  );
    }

    // I 2 metodi seguenti vengono usati (solo) da utente?
    void set_input(Tin i){ //TODO: maybe erase ?
        while(!is_ready());
        *input_ptr = i;
	    new_input = true;
    }

    void set_input_ptr(void* in_ptr){
        while(!is_ready());
        input_ptr = static_cast<Tin*>(in_ptr);
	new_input = true;
    }

    void set_output_ptr(Tout* optr){
        output_ptr = optr;
    }

    void* get_input_ptr(){
        return input_ptr;
    }

    void* get_output_ptr(){
        return output_ptr;
    }

    Tout get_output(){
        return *output_ptr;
    }

    bool is_ready(){
        return !new_input;
    }   

    void set_new_input(){
        new_input=true;
    }

    void add_next(Node &n, bool=false){
        next = &n;
	output_ptr = static_cast<Tout*>(n.get_input_ptr());      
    }

    int num_nodes(){
        return 1;
    }

    void collapse(){
        while(new_input); //collapse only after the input has been executed	
        collapsed=-1;
    }

    bool is_collapsed(){
        return collapsed==-1;
    }

    void collapse_next_stage(){ //TODO: parte difficile va qui, next può non essere chiaro
	IStage * nptr = static_cast<IStage*>(next);
	for(int i=0; i< collapsed; i++){	  
	    nptr = nptr->get_next();
	}	
	//Ends the thread, but only after finishing processing current task
	collapsed += nptr->num_collapsed(); //TODO: controllo che ritorni >=0
	nptr->collapse();
	collapsed++; 
	cout << "Stage # " << i << " has collapsed the successive Stage. It will manage " << 
		collapsed << " Stages now" << endl;
    }

    IStage* get_next(){
        return static_cast<IStage*>(next);
    }

    double get_exec_time(){
        return exec_time;
    }

    int num_collapsed(){
        return collapsed;
    }

    Tf fun;
    Tout * output_ptr;
    Tin * input_ptr;
    vector<thread> threads; //TODO: puoi usare unique_ptr o shared_ptr 
    bool new_input;
    Node * next;
    int collapsed;
    int const i; //for debug
    double exec_time;
};

