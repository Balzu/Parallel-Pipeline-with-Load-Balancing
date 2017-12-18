#include "Node.hpp"
#include <iostream>
#include <thread>
#include <vector>
using namespace std;;

struct IStage : Node{
    virtual void stage_func() = 0;
    Node * next;
    virtual void collapse() = 0;
    virtual IStage* get_next() = 0;
    
};

template <typename Tin, typename Tf, typename Tout>
struct Stage : IStage{

    Stage(Tf  function, int ind):fun{function}, input_ptr{new(Tin)},output_ptr{new(Tout)},
	    /*{false},*/ next{nullptr}, new_input{false}, collapsed{0}, i{ind}{};	
//TODO: invece di dire che sono ready solo quando non sto calcolando, posso dire che sono ready quando ho spazio in input
   
    void stage_func(){ 
	cout << "Input ptr: " << input_ptr << endl; 
	Tin input = *input_ptr;
        Tout out = fun(*input_ptr);  // compute-intensive line..
        cout << "Stage computed: " << out << endl;
	if (next!=nullptr)  // wait for next node to be ready only if next node exists
	    while(!(next->is_ready())){};
	*output_ptr = out;
	if(next!=nullptr) next->set_new_input();
        new_input = false;        	
	return;
    }

    void run_thread(){
        while(!end()){ 
            while(!end() && !new_input){}; //spinning..    
            if(end()){
                if(next!=nullptr && collapsed!=-1){ //collapsed = -1 means to stop only this thread (because of collapsing) 
	 	    /*IStage * n = dynamic_cast<IStage*>(next);
                    n->set_input_ptr(nullptr);
		    for(int i=0; i<collapsed; i++){
		        n = dynamic_cast<IStage*>(n->next);
			n->set_input_ptr(nullptr);
		    }*/
		    IStage * nptr = static_cast<IStage*>(next);
		    nptr -> set_input_ptr(nullptr);
		    for(int i=0; i< collapsed; i++){	  
	 	        nptr = nptr->get_next();
			nptr->set_input_ptr(nullptr);
		    }	
		}
            }
            else{
	        cout << "Processing an item\n";
	        stage_func();
		if(collapsed!=0){ //this thread has to run more Stages TODO: puoi togliere if
		    IStage * nptr = static_cast<IStage*> (next);
		    for(int i=0; i< collapsed; i++){
		        nptr->stage_func();
			nptr = nptr->get_next();
		    }
		}
            }
    	}
    }

    void run(){
        thread t(&Stage::run_thread, this); 
       // *tptr = move(t);
        //t.detach();
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
        collapsed=-1;
    }

    void collapse_next_stage(){ //TODO: parte difficile va qui, next può non essere chiaro
	IStage * nptr = static_cast<IStage*>(next);
	for(int i=0; i< collapsed; i++){	  
	    nptr = nptr->get_next();
	}	
	//Ends the thread, but only after finishing processing current task
	nptr->collapse();
	collapsed++; 
    }
/*
    void end_s(){ //TODO: usa valore speciale in input per terminazione
        end=true;
    }*/

    IStage* get_next(){
        return static_cast<IStage*>(next);
    }

    Tf fun;
    Tout * output_ptr;
    Tin * input_ptr;
  //  bool end;
    vector<thread> threads; //TODO: puoi usare unique_ptr o shared_ptr 
    bool new_input;
    Node * next;
    int collapsed;
    int i; //for debug
};






