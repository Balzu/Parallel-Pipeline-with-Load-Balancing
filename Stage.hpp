#include "Node.hpp"
#include <iostream>
using namespace std;;

template <typename Tin, typename Tf, typename Tout>
struct Stage : Node{

    Stage(Tf  function):fun{function}, input_ptr{new(Tin)},output_ptr{new(Tout)},
	    end{false}, next{nullptr}, new_input{false},ready{true}{};	
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

    void run(){
//	while(!end){ //TODO: non bella questa condizione
 	    while(!end && !new_input){}; //spinning..    
	//while(!ready);
            if(end) return;
	    cout << "Processing an item\n";
	    stage_func();
//	}
    }

    void run_and_wait_end(){
        cout << "Running node and waiting end\n";
    }

    void set_input(Tin i){ //TODO: maybe erase ?
        *input_ptr = i;
	new_input = true;
    }

    void set_input_ptr(void* in_ptr){
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
        return output_ptr;
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

    Tf fun;
    Tout * output_ptr;
    Tin * input_ptr;
    bool end;
    bool ready; //ready to accept a new task
    bool new_input;
    Node * next;
};

template <typename Tin, typename Tout>
class Function {    
    Tout operator()(Tin);
};

