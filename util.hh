#include <cstdio>
#include <sys/time.h>

inline double gettime(){
  timeval tv;
  gettimeofday(&tv,NULL);
  return static_cast<double>(tv.tv_sec)+static_cast<double>(tv.tv_usec)/1000000.0;
}

class FileData {
public:
  FileData(const char* filepath) : m_data(NULL) {
    unsigned long size;
    FILE* f = fopen(filepath, "r");
    if(!f)
      return;

    if(fseek(f, 0, SEEK_END)!=0)
      goto end;
    
    size = ftell(f);
    if(fseek(f, 0, SEEK_SET)!=0)
      goto end;
    
    m_data = new char[size+1];
    if(fread(m_data, sizeof(char), size, f) != size) {
      delete [] m_data;
      m_data = NULL;
      goto end;
    }
    m_data[size] = '\0';
    m_size = static_cast<unsigned>(size);
    
  end:
    fclose(f);
  }

  ~FileData() {
    delete [] m_data;
  }

  operator bool() const { return m_data!=NULL; }
  const char* c_str() const { return m_data; }
  unsigned size() const { return m_size; }
  
private:
  char* m_data;
  unsigned m_size;
};
