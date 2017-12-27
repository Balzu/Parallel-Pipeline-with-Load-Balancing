#include <mutex>
#include <queue>
#include <vector>
#include <iostream>
#include <atomic>
#include <climits>
#include <thread>
using namespace std;

// TODO: Deve essere thread safe perchè in ogni
// momento la coda ha un produttore e un consumatore

template<typename T>
struct Comparator{
    bool operator()(pair<T,int> p1, pair<T,int> p2){
        return p1.second > p2.second;
    }
};

//Thread-Safe Ordered Heap
template<typename T>
struct TSOHeap{

    TSOHeap(int _max=10):id{0},size{0},max{_max}{};

    // Consumer never stuck
    void push(T& item){
//Don't use condition variable because don't want to deschedule threads
	while(size==max); //spinning..
	{//grant exclusive access to the heap
	    lock_guard<mutex> lock(heap_mutex);
	    if(item!=nullptr)//TODO anche 0 è considerato NULL, non va bene
		heap.push(pair<T,int>(item, ++id));
	    else{	    
		heap.push(pair<T,int>(item, INT_MAX));
		id++;
	    }
	    size++;
	}        
    }
    
    T pop(){
        while(size==0);
	{
	    lock_guard<mutex> lock(heap_mutex);
	    T elem = heap.top().first;
	    heap.pop();
	    size--;
	    return elem;
	}
    }

    priority_queue<pair<T,int>, vector<pair<T,int>>,Comparator<T>> heap;
    atomic<int> id;
    atomic<int> size;
    int max;
    mutex heap_mutex;
    
};

template<typename T>
void insert_heap(TSOHeap<T>& heap, int item){
    heap.push(item);
}

int main(int argc, char* argv[]){
    TSOHeap<int> h1 {110};
    vector<thread> vt;
    for(int i=0; i<100; i++){
        vt.push_back(thread(insert_heap<int>, ref(h1), i));
    }
    for(int i=0; i<100; i++){
        vt[i].join();
    }
    while(h1.size!=0){
        cout << h1.pop() << " ";
    }
    cout << endl;
    return 0;
   
    /* while(h1.heap.empty()==false){
        cout << h1.heap.top() << "";
	h1.heap.pop();
    }
    cout << endl;
*/
    

}
