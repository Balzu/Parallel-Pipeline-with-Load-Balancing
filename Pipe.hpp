#include "Stage.hpp"
#include <list>
#include <thread>
#include <algorithm>
using namespace std;;

template <typename Tin, typename Tout>
struct Pipe : Node {
    template<typename Head, typename... Tail> 
    Pipe(Head h, Tail... t):max_consec{2},consecutive{0},slowest{-1},end{false} {
        create_pipeline(h,t...);
    }
    
    template<typename Head, typename... Tail> 
    void create_pipeline(Head h, Tail... t){
        add_node(h);
	create_pipeline(t...);
    }

   
    void add_next(Node& next, bool outer=true){
        nodes.back()->add_next(next, false);
	IStage * isptr = static_cast<IStage*>(&next);
	nodes.push_back(isptr);  		
    } 

    void add_node(IStage* head_ptr){
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

    void set_input(void * in_ptr){
        nodes.front()->set_input(in_ptr);
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
    }

    void run(list<Tin>&& input){
        thread t(&Pipe::run_manager, this, ref(input));
	while(!end)
	    monitor_times();//monitora lui, che altrimenti sarebbe in attesa
	t.join();
    }

    void run_manager(list<Tin>& input){
       run();
       for(auto& x:input){
           set_input(&x);
	 //  monitor_times();
       }
   
       set_input(nullptr);
       for(auto& s : nodes)
	   s->wait_end();
       end=true;
    }

   
    void run_and_wait_end() {}//TODO

    
    void monitor_times(){
        vector<pair<int,double>> times;
	
	for (int i=0; i<nodes.size(); i++){	    
	    if(!(nodes[i]->is_collapsed())) //Can collapse only stages not already collapsed 
		times.push_back(make_pair(i, nodes[i]->get_exec_time()));
	}
	if (times.size()>=3){ // can't collapse stages if you don't have at least three
	    auto pair_comp = [](pair<int,double>p1, pair<int,double>p2){
	        return p1.second < p2.second;
  	    };	
	    sort(times.begin(), times.end(), pair_comp);
	    if(times[0].second==0) return; //not all stages executed
	    if(times.back().first==slowest){ //slowest node is the same one of before
	        if (++consecutive>=max_consec){ //have to LOAD BALANCE
		    double max_exec_time = times.back().second;
		    for(int i=0; i<times.size(); i++){ //times sorted by increasing times
		        int j = times[i].first;
			try{  //TODO: bug possono essere qui
			    if(nodes.at(j)->get_exec_time() + nodes.at(j+1)->get_exec_time() < max_exec_time){
			        int n = nodes.size()-1;
				if (j + nodes[j]->num_collapsed() < n){
				    nodes[j]->collapse_next_stage();
			            return;
				}
			    }
			}
			catch(out_of_range){/*just skip this iteration*/}
		    }		
	         }
	    }	
	    else
	        slowest = times.back().first;
	}
    }


    int slowest;
    int consecutive;
    int const max_consec;
    bool end;
    bool ready;
    bool new_input;
    IStage* next;
    vector<IStage*> nodes;
};




