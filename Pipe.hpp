#include "Stage.hpp"
#include <list>
#include <thread>
#include <algorithm>
using namespace std;;

template <typename Tin, typename Tout>
struct Pipe : Node {
    template<typename Head, typename... Tail> 
    Pipe(Head h, Tail... t):max_consec{2},consecutive{0},slowest{-1},end{false},num_samples{2}
    {
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
	    monitor_times2();//monitora lui, che altrimenti sarebbe in attesa
	t.join();
    }

    void run_manager(list<Tin>& input){
       run();
       for(auto& x:input){
	//   this_thread::sleep_for(chrono::milliseconds(10));
           set_input(&x);
	 //  monitor_times();
       }
   
       set_input(nullptr);
       for(auto& s : nodes)
	   s->wait_end();
       end=true;
       cout << "end=true" << endl;
    }

   
    void run_and_wait_end() {}//TODO

    void monitor_times2(){
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
	    for(int i=0; i<nodes.size(); i++){
	        if(nodes[i]->get_exec_time()!=0){ //TODO: e !=collapsed?
		    pair<double,int> measure = pair<double,int>(nodes[i]->get_exec_time(),++count[i]);
		    nodes[i]->reset_exec_time();
		    measures[i].push(measure);
		    if(count[i]<=num_samples){
		        avg[i] = (avg[i]*(count[i]-1) + measure.first) / count[i];
		    }
		    else{
		        double old = measures[i].top().first; //TODO: controlla che sequenza estrazioni
			// sia nel corretto ordine (print)
                        //ordinamento heap garantisce che elimini la misurazione più vecchia	
			measures[i].pop(); 
			avg[i] = (avg[i] * num_samples - old + measure.first) / num_samples; //running avg 
		    }
		  //  cout << "avg[ " << i << " ] = " << avg[i] << " , count = " << count[i] << endl;

		}	    
	    }
	    if(is_steady_state(count)){
		cout << "STEADY STATE " << endl;
	        int slowest = get_slowest_stage(avg);
		cout << "slowest: " << slowest << endl;
		for(int i=0; i<nodes.size()-1; i++){
		    if(avg[i]+avg[i+1]<avg[slowest]){
		        if(!(nodes[i]->is_collapsed()) && !(nodes[i+1]->is_collapsed())){
			    nodes[i]->collapse_next_stage();
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
    int num_samples;
    IStage* next;
    vector<IStage*> nodes;
};




