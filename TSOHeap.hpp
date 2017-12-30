#include <mutex>
#include <queue>
#include <vector>
#include <atomic>
#include <climits>

using namespace std;

template<typename T>
struct Comparator{
    bool operator()(pair<T,int> p1, pair<T,int> p2){
        return p1.second > p2.second;
    }
};

//Thread-Safe Ordered Heap
template<typename T>
struct TSOHeap{

    TSOHeap(int _max=10):size{0},max{_max}{};

    // It is the caller that states the ordering, providing the id
    // (ordering can't be maintained inside this class, because of non determinism
    // due to multiple threads calling this method)
    void push(T* item, int id){
//Don't use condition variable because don't want to deschedule threads
	while(size==max);
	{
	    lock_guard<mutex> lock(heap_mutex);
            heap.push(pair<T*,int>(item, id));
	    size++;
	}        
    }
    
    pair<T*,int> pop(){
        while(size==0);
	{
	    lock_guard<mutex> lock(heap_mutex);
	    pair<T*,int> p = heap.top();
	    heap.pop();
	    size--;
	    return p;
	}
    }

    ~TSOHeap(){cout<<"DESTRUCTOR"<<endl;}//TODO: put variable to see wheter invoked twice, or delete using a method

    priority_queue<pair<T*,int>, vector<pair<T*,int>>,Comparator<T*>> heap;
    atomic<int> size;
    int max;
    mutex heap_mutex;    
};

