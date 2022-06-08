Implemented iterators that work in circulation, meaning ++end() leads to begin(), and --begin() leads to the last element (not end()), like STL's link<Elem> iterators.
Represented end() with pointer with value 0, to save memory used for the end() element.
