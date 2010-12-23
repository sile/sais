#include <cstdio>
#include <sys/time.h>

inline double gettime(){
  timeval tv;
  gettimeofday(&tv,NULL);
  return static_cast<double>(tv.tv_sec)+static_cast<double>(tv.tv_usec)/1000000.0;
}

class FileData {
public:
  FileData(const char* filepath) : data(NULL) {
    unsigned long size;
    FILE* f = fopen(filepath, "r");
    if(!f)
      return;

    if(fseek(f, 0, SEEK_END)!=0)
      goto end;
    
    size = ftell(f);
    if(fseek(f, 0, SEEK_SET)!=0)
      goto end;
    
    data = new char[size+1];
    if(fread(data, sizeof(char), size, f) != size) {
      delete [] data;
      data = NULL;
      goto end;
    }
    data[size] = '\0';
    
  end:
    fclose(f);
  }

  ~FileData() {
    delete [] data;
  }

  operator bool() const {
    return data!=NULL;
  }

  const char* c_str() const {
    return data;
  }
  
private:
  char* data;
};
