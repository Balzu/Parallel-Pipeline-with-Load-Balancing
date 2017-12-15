
struct Node{
    virtual void run() = 0;
    virtual void run_and_wait_end() = 0;
    virtual int num_nodes() = 0;
    virtual void add_next(Node&, bool=false)=0; 
    virtual ~Node() {}
    virtual bool is_ready() = 0;
    virtual void set_new_input() = 0; 
    virtual void* get_input_ptr() = 0;
    virtual void* get_output_ptr() = 0;
    virtual void set_input_ptr(void*) = 0;
};

/*
template<typename Tin>
struct Node_it : Node{
    virtual Tin* get_input_ptr() = 0;
};


template<typename Tin, typename Tout>
struct Node_t : Node{
    virtual void set_input(Tin) = 0;
    virtual Tout get_output() = 0;   
};
*/

