Self-implemented allocator<T>, that is used to separate initialization from allocation, and destrunction from deallocation, which is not possible with [new, delete] operator. 
The self-implemented allocator, My_Allocator<T>, imitates the behavior of std::allocator<T>.
