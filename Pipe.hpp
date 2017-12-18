#include "Stage.hpp"
#include <list>
#include <thread>
using namespace std;;

template <typename Tin, typename Tout>
struct Pipe : Node {
    template<typename Head, typename... Tail> 
    Pipe(Head h, Tail... t){
        create_pipeline(h,t...);
    }
    
    template<typename Head, typename... Tail> 
    void create_pipeline(Head h, Tail... t){
        add_node(h);
	create_pipeline(t...);
    }

    //Invoca metodo ricorsivamente finchè l'ultimo elemento di nodes è uno stage
    void add_next(Node& next, bool outer=true){
        nodes.back()->add_next(next, false);
	if(outer==true) nodes.push_back(&next);  		
    } 

    void add_node(Node* head_ptr){
	if(!nodes.empty())  // if not empty link the two stages 
	    add_next(*head_ptr,true); //invoke the method of the pipeline	    
	else 
	    nodes.push_back(head_ptr);
    }

    void create_pipeline () {}

    void* get_input_ptr(){
        return nodes.front()->get_input_ptr();
    }

    void* get_output_ptr(){
        return nodes.back()->get_output_ptr();
    }

    void set_input_ptr(void * in_ptr){
        nodes.front()->set_input_ptr(in_ptr);
    }

    void set_input(Tin in){
        nodes.front()->set_input_ptr(&in);
    }

 //   Tout get_output(){
 //       return nodes.back()->get_output();
 //   }

    void set_new_input(){
        nodes.front()->set_new_input();
    }

    bool is_ready(){
        nodes.front()->is_ready();
    }
    

    int num_nodes(){
	int c= 0;
        for(auto &x : nodes)
	    c += x->num_nodes();
	return c;
    }

    void run(){
        for(auto &x: nodes)
	    x->run();
    }//TODO

    void run(list<Tin>& input){
        thread t(&Pipe::run_manager, this, ref(input)); 
    }

    void run_manager(list<Tin>& input){
        
    }

   
    void run_and_wait_end() {}//TODO


//TODO: puoi togliere attributi input_ptr e output_ptr, e mettere invece un metodo
//che  restituisce come input_ptr della pipeline l'input_ptr del primo stage
//e come output_ptr l'output_ptr dell' ultimo stage    
 //   Tout * output_ptr; 
 //   Tin * input_ptr;
    bool end;
    bool ready;
    bool new_input;
    Node* next;
    list<Node*> nodes;
};
