C++ has its own file handler, std::fstream, but I implemented my file handler. 
That file handler uses C-style file handlers like FILE*, fopen(), etc, and achieves the RAII (Resource Acquisition Is Initialization) concept.

