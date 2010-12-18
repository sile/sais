#include <iostream>

#include "sais.hh"
#include "mmap_t.hh"

int main(int argc, char** argv) {
    MPHF::mmap_t m(argv[1]);
    if(m) {
        std::cerr << "input size: " << m.size << std::endl;
        
        const char* text = (const char*)m.ptr; // XXX: null terminated?
        SA_IS sais(text);
        sais.construct();
    }
    return 0;
}

