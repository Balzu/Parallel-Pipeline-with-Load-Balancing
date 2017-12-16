#include "Node.hpp"
#include <iostream>
#include <thread>
#include <vector>
using namespace std;;

template <typename Tin, typename Tf, typename Tout>
struct Stage : Node{

    Stage(Tf  function):fun{function}, input_ptr{new(Tin)},output_ptr{new(Tout)},
	    end{false}, next{nullptr}, new_input{false}{};	
//TODO: invece di dire che sono ready solo qando non sto calcolando, posso dire che sono ready quando ho spazio in input
    void stage_func(){  
	Tin input = *input_ptr;
      //  ready = false; //TODO: atomic set
    Tout out = fun(*input_ptr);  // compute-intensive line..
	if (next!=nullptr)  // wait for next node to be ready only if next node exists
	    while(!(next->is_ready())){};//TODO: and end==false 
	*output_ptr = out;
	if(next!=nullptr) next->set_new_input();
	//ready = true;
    new_input = false;        	
	return;
    }

    void run_thread(){
        while(!end){ //TODO: non bella questa condizione
 	        while(!end && !new_input){}; //spinning..    
	//while(!ready);
            if(end) return;
            if (input_ptr==nullptr) cout << "OOOOOOOO" << endl;
	        cout << "Processing an item\n";
	        stage_func();
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

    void end_s(){ //TODO: usa valore speciale in input per terminazione
        end=true;
    }

    Tf fun;
    Tout * output_ptr;
    Tin * input_ptr;
    bool end;
    vector<thread> threads; 
    bool new_input;
    Node * next;
};

template <typename Tin, typename Tout>
class Function {    
    Tout operator()(Tin);
};

