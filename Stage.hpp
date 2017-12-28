#include "Node.hpp"
#include "TSOHeap.hpp"
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
    virtual void reset_exec_time()=0; 
    virtual void collapse_next_stage() = 0;
    virtual void wait_end() = 0;
    virtual int num_collapsed() = 0;
};

template <typename Tin, typename Tf, typename Tout>
struct Stage : IStage{

    Stage(Tf  function, int ind):fun{function}, input_ptr{new(TSOHeap<Tin>)},_end{false},
	     next{nullptr}, collapsed{0}, i{ind}, exec_time{0.0},count{0},collapsing{false}{};	
   
    void stage_func(){ 
	pair<Tin*,int> input_pair = input_ptr->pop();
	Tin * input = input_pair.first;
	int id = input_pair.second;
	if (input!=nullptr){
            auto start = chrono::system_clock::now();
        //    cout << "t " << i << " IN: e = " << *input << ", id = " << id << endl; 
	    Tout out = fun(*input);  // compute-intensive line..
            auto end = chrono::system_clock::now();
	    chrono::duration<double> diff = end-start;
	    exec_time = diff.count(); 
	    if (next!=nullptr){
		next->set_input(new Tout(out));
	       // output_ptr->push(new Tout(out), id);//TODO: memory leak	
              //  cout << "t " << i << " OUT: e = " << out << ", id = " << id << endl; 
	    }
	}
	else{
	//nullptr indica che devi finire(end=true). Poi usi variabile collapsed per 
	//discriminare se devi finire perchè collassato o perchè la computazione è finita
	    _end = true;   
	}
    }

    void run_thread(){
        while(!end()){
	    while(collapsing);		
	    stage_func();
	    if(collapsed>0){ //this thread has to run more Stages
	        IStage * nptr = static_cast<IStage*> (next);
		for(int i=0; i< collapsed; i++){
		    nptr->stage_func();
		    nptr = nptr->get_next();
		}
		cout << "Run collapsed function" << endl;
	    }            
    	} //Finalization..       
        if(collapsed!=-1){ //collapsed = -1 means to stop only this thread
	// If the next stages are collapsed, then they are already ended. 
	    IStage * nptr = static_cast<IStage*>(next);		 
	    while(nptr!=nullptr && nptr->is_collapsed())	  
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
        return (_end || collapsed==-1  );
    }

    // I 2 metodi seguenti vengono usati (solo) da utente?
    void set_input(void * iptr){ //TODO: maybe erase ?
       // while(!is_ready());
        input_ptr->push(static_cast<Tin*>(iptr), ++count);//TODO cast a Tin* ?

    }

    /*
    void set_input_ptr(void* in_ptr){
        while(!is_ready()); //TODO: non più necessario questo metodo
        input_ptr = static_cast<TSOHeap<Tin>*>(in_ptr);
	new_input = true;
    }
    */

    void set_output_ptr(TSOHeap<Tout>* optr){
        output_ptr = optr;
    }

    void* get_input_ptr(){
        return input_ptr;
    }

    void* get_output_ptr(){
        return output_ptr;
    }

    Tout get_output(){//TODO
        return *output_ptr;
    }

    void add_next(Node &n, bool=false){
        next = &n;
	output_ptr = static_cast<TSOHeap<Tout>*>(n.get_input_ptr());      
    }

    int num_nodes(){
        return 1;
    }

    void collapse(){       
        collapsed=-1;
        input_ptr->push(nullptr, INT_MIN);//TODO cast a Tin* ?        	
	while((input_ptr->size)>0);        
    }

    bool is_collapsed(){
        return collapsed==-1;
    }

    void collapse_next_stage(){ //TODO: parte difficile va qui, next può non essere chiaro
	collapsing = true;
	IStage * nptr = static_cast<IStage*>(next);


	for(int i=0; i< collapsed; i++){ //TODO rimuovi questa parte	  
	    nptr = nptr->get_next();
	}	
	//Ends the thread, but only after finishing processing current task
	collapsed += nptr->num_collapsed(); //TODO: controllo che ritorni >=0
	collapsed++;


	nptr->collapse();
	collapsing = false; 
	cout << "Stage # " << i << " has collapsed the successive Stage. It will manage " << 
		collapsed << " Stages now" << endl;
    }

    IStage* get_next(){
        return static_cast<IStage*>(next);
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
    vector<thread> threads; //TODO: puoi usare unique_ptr o shared_ptr 
    bool new_input;
    Node * next;
    bool _end;
    int collapsed;
    int const i; //for debug
    double exec_time;
    int count;
    bool collapsing;
};

