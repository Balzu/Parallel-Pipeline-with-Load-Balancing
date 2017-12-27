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

    TSOHeap(int _max=10):size{0},max{_max}{};

    // Consumer never stuck
    // It is the caller that states the ordering, providing the id
    // (ordering can't be maintained inside this class, because of non determinism
    // due to multiple threads calling this method)
    void push(T* item, int id){
//Don't use condition variable because don't want to deschedule threads
	while(size==max); //spinning..
	{//grant exclusive access to the heap
	    lock_guard<mutex> lock(heap_mutex);
	    if(item!=nullptr)
		heap.push(pair<T*,int>(item, id));
	    else	    
		heap.push(pair<T*,int>(item, INT_MIN));
	    size++;
	}        
    }
    
    T pop(){
        while(size==0);
	{
	    lock_guard<mutex> lock(heap_mutex);
	    T * elem = heap.top().first;
	    heap.pop();
	    size--;
	    return *elem;
	}
    }

    priority_queue<pair<T*,int>, vector<pair<T*,int>>,Comparator<T*>> heap;

    atomic<int> size;
    int max;
    mutex heap_mutex;
    
};

template<typename T>
void insert_heap(TSOHeap<T>& heap, int * item, int id){
    heap.push(item, id);
}

int main(int argc, char* argv[]){
    TSOHeap<int> h1 {110};
    vector<thread> vt;
    for(int i=0; i<100; i++){
        vt.push_back(thread(insert_heap<int>, ref(h1), new int(i), i));
    }
    for(int i=0; i<100; i++){
        vt[i].join();
    }
    while(h1.size!=0){
        cout << h1.pop() << " " ;
    }
    cout << endl;
    return 0;    

}
