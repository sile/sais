#include <iostream>
#include "sais.hh"
#include "util.hh"

int main(int argc, char** argv) {
  if(argc != 2) {
    std::cerr << "Usage: sais FILEPATH" << std::endl;
    return 1;
  }
  
  double beg_t = gettime();
  FileData fd(argv[1]);
  if(!fd) {
    std::cerr << "Can't open file: " << argv[1] << std::endl;
    return 1;
  }
  std::cerr << "Read file data:\t\t took " << gettime()-beg_t << " sec" << std::endl;
  
  beg_t = gettime();
  const char* text = fd.c_str();
  SAIS sais(text);
  std::cerr << "Construct Suffix-Array:\t took " << gettime()-beg_t << " sec" << std::endl;

  beg_t = gettime();
  for(unsigned i=0; i < sais.size(); i++)
    std::cout << sais.sa()[i] << std::endl;
  std::cerr << "Print SA elements:\t took " << gettime()-beg_t << " sec" << std::endl;
  
  return 0;
}
