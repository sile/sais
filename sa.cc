#include <cstdlib>
#include "sa.hh"
#include "mmap_t.hh"
#include <iostream>

// find the suffix array SA of s[0..n-1] in {1..K}^n
// require s[n-1]=0 (the sentinel!), n>=2
// use a working space (excluding s and SA) of at most 2.25n+O(1) for a constant alphabet

int main(int argc, char** argv) {
    MPHF::mmap_t m(argv[1]);
    if(m) {
        std::cerr << "input size: " << m.size << std::endl;
        int* SA = new int[m.size+1];
        SA_IS((unsigned char*)m.ptr, SA, m.size+1, 0x100, sizeof(char));

        for(unsigned i=0; i < m.size; i++)
          std::cout << SA[i+1] << std::endl;
    }
    return 0;
}
