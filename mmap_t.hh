#ifndef MPHF_MMAP_T_HH
#define MPHF_MMAP_T_HH

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

namespace MPHF {
  struct mmap_t {
    mmap_t(const char* path, bool write_mode=false, int flags=MAP_SHARED){
      int OPEN_MODE=O_RDONLY;
      int PROT = PROT_READ;
      if(write_mode) {
	OPEN_MODE=O_RDWR;
	PROT |= PROT_WRITE;
      }
      
      int f = open(path, OPEN_MODE);
      struct stat statbuf;
      fstat(f, &statbuf);
      ptr = mmap(0, statbuf.st_size, PROT, flags, f, 0);
      size=statbuf.st_size;
      close(f);  
    }
    
    ~mmap_t() { munmap(ptr, size); }
    
    operator bool () const { return ptr!=reinterpret_cast<void*>(-1); }
    
    size_t size;
    void *ptr;
  };
}

#endif 
