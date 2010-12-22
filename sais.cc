#include <iostream>

#include "sais.hh"
#include "mmap_t.hh"
#include <cstring>

class filedata {
public:
  filedata(const char* filepath) : data(NULL){
    MPHF::mmap_t m(filepath);
    if(m) {
      data = new char[m.size+1];
      memcpy(data, m.ptr, m.size);
      data[m.size] = 0;
    }
  }

  ~filedata() {
    delete [] data;
  }
  
  char* data;
};

int main(int argc, char** argv) {
  filedata fd(argv[1]);
  //std::cerr << "input size: " << m.size << std::endl;
  
  const char* text = fd.data;
  SA_IS sais(text);

  /*
  for(unsigned i=0; i < sais.size(); i++)
    std::cout << sais.sa()[i] << std::endl;
  */

  return 0;
}

