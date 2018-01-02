#include "Stage.hpp"
#include <list>
#include <thread>
#include <algorithm>
using namespace std;;

template <typename Tin, typename Tout>
struct Pipe{
   
    Pipe(list<IStage*>li, int n_samples=10):slowest{-1},end{false},num_samples{n_samples}
    {
        for(auto& s:li)
	    add_node(s);
    }

    void add_node(IStage* head_ptr){
	if(!nodes.empty()) 
	    nodes.back()->add_next(*head_ptr); 
        nodes.push_back(head_ptr);
    }

    void create_pipeline () {}

    void set_input(void * in_ptr){
        nodes.front()->set_input(in_ptr);
    }

    int num_nodes(){
	return nodes.size();
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
       for(auto& x:input)
           set_input(&x);   
       set_input(nullptr);
       end=true;
       for(auto& s : nodes)
	   s->wait_end();
    } 

    void monitor_times(){
	// initialization phase
        vector<int> count;
	vector<double> avg; 
	vector<priority_queue<pair<double,int>, vector<pair<double,int>>,Comparator<double>>> measures;
	for(auto&x : nodes){
	    count.push_back(0);
	    avg.push_back(0);
	    measures.push_back(priority_queue<pair<double,int>, 
	            vector<pair<double,int>>,Comparator<double>>());
	}
	while(!end){
	// monitoring phase	
	    for(int i=0; i<nodes.size(); i++){
	        if(nodes[i]->get_exec_time()!=0){ 
		    pair<double,int> measure = pair<double,int>(nodes[i]->get_exec_time(),++count[i]);
		    nodes[i]->reset_exec_time();
		    measures[i].push(measure);
		    if(count[i]<=num_samples){
		        avg[i] = (avg[i]*(count[i]-1) + measure.first) / count[i];
		    }
		    else{
		        double old = measures[i].top().first; 
                        //ordinamento heap garantisce che elimini la misurazione più vecchia	
			measures[i].pop(); 
			avg[i] = (avg[i] * num_samples - old + measure.first) / num_samples; //running avg 
		    }
		}	    
	    }
	    // updating phase
	    if(is_steady_state(count)){
	        int slowest = get_slowest_stage(avg);
		for(int i=0; i<nodes.size()-1; i++){
		    if(avg[i]+avg[i+1]<avg[slowest]){
		        if(nodes[i]->num_collapsed()==0 && nodes[i+1]->num_collapsed()==0){
			    nodes[i]->collapse_next_stage();
			    cout << "Slowest: " << slowest << " , time = " << avg[slowest]
				    << " num measured = " << count[slowest] << endl;
			    cout << "Fast 1: " << i << " , time = " << avg[i]
				    << " num measured = " << count[i] << endl;
			    cout << "Fast 2: " << (i+1) << " , time = " << avg[i+1]
				    << " num measured = " << count[i+1] << endl;
			    break;    
			}		    
		    }		
		}	    
	    }
	}   
    }

    bool is_steady_state(vector<int>& count){
        for(auto& c: count){
	    if(c<num_samples) return false;
	}
	return true;    
    }

    int get_slowest_stage(vector<double>& avg){
        double max = 0.0;
	int index = -1;
	for(int i=0; i<avg.size(); i++){
	    if(avg[i]>max){
	        max=avg[i];
		index = i;
	    }
	}
	return index;
    }

    int slowest;
    bool end;
    int num_samples;
    IStage* next;
    vector<IStage*> nodes;
};
