#ifndef SA_IS_HH
#define SA_IS_HH

#include <vector>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <string>
typedef std::vector<bool> flags;

struct Index {
  void set(unsigned start, unsigned s_cur) {
    this->start = start;
    this->l_cur = start;
    this->s_cur = s_cur;
  }
    
  unsigned start;
  unsigned l_cur;
  unsigned s_cur;
};

template <typename T>
struct Buckets {
  Buckets(const T* beg, const T* end) : buf(NULL) {
    unsigned limit = *std::max_element(beg, end)+1;
    idx.resize(limit+1);
    std::vector<unsigned> freq(limit, 0);
    
    for(const T* c=beg; c!=end; c++)
      ++freq[*c];
        
    unsigned offset=0;
    for(unsigned i=0; i < idx.size(); i++) {
      idx[i].set(offset, offset+freq[i]-1);
      offset += freq[i];
    }
    idx[limit].set(offset,offset);
        
    size = end-beg;
    buf = new unsigned[size];
    memset(buf,0xFF,size*sizeof(unsigned));
  }

  ~Buckets() {
    delete [] buf;
  }

  void initS() {
    for(unsigned i=0; i < idx.size(); i++)
      idx[i].s_cur = idx[i+1].start-1;
  }

  void init() {
    for(unsigned i=0; i < idx.size(); i++) 
      idx[i].set(idx[i].start,idx[i+1].start-1);
    memset(buf,0xFF,size*sizeof(unsigned));
  }

  void putL(T c, unsigned pos) {
    buf[idx[c].l_cur++] = pos;
  }

  void putS(T c, unsigned pos) {
    buf[idx[c].s_cur--] = pos;
  }
    
  // XXX:
  int position(unsigned i) const {
    return (int)buf[i];
  }

public:
  std::vector<Index> idx;
  unsigned* buf;
  unsigned size;
};

typedef std::vector<unsigned> lms_ary_t;

class SA_IS {
public:
  SA_IS(const char* text) : source((const unsigned char*)text) {
        
  }    

  void construct() {
    Buckets<unsigned char> bkt(source, source+strlen((const char*)source)+1);
    impl<unsigned char>(source, source+strlen((const char*)source)+1, bkt);
    
    /*
    std::string s((const char*) source);
    for(int i=0; i < bkt.size-1; i++) {
      if(strcmp(((const char*) source)+bkt.buf[i], ((const char*) source)+bkt.buf[i+1]) >= 0) {
        std::cerr << i << "# not ordered!" << std::endl;

        unsigned match_len = 0;
        for(;; match_len++) {
          if(s[bkt.buf[i]+match_len] != s[bkt.buf[i+1]+match_len])
            break;
        }
        std::cerr << s.substr(bkt.buf[i], match_len+10) << std::endl;
        std::cerr << s.substr(bkt.buf[i+1], match_len+10) << std::endl;
      }
    }
    */
  }

private:
  const unsigned char* source;
    
  template<typename T>
  struct Cmp {
    const T* src;
    Cmp(const T* src) : src(src) {}
    bool operator()(unsigned l, unsigned r) const {
      if(l==r)
        return false;
      
      for(unsigned i1=l,i2=r;; i1++, i2++) {
        if(src[i1] < src[i2])
          return true;
        if(src[i1] > src[i2])
          return false;
      }
    }
  };

private:
  template <typename T>
  void impl(const T* beg, const T* end, Buckets<T>& bkt) {
    std::cerr << "IN: " << (end-beg) << std::endl;

    flags types(end-beg, 0);
    classify(beg, types);
    lms_ary_t lms_ary;
    calc_lms_ary(types, lms_ary);
    induce(beg, types, lms_ary, bkt);

    std::vector<unsigned> s1(types.size()/2+1,(unsigned)-1);
    const bool uniq = reduce(beg, types, bkt, lms_ary.size(), s1);
    if(uniq) {
      for(std::size_t i = 0; i < s1.size(); i++)
        s1[i] = lms_ary[s1[i]];
      bkt.init();
      induce(beg, types, s1, bkt, true);

      // 
      //Cmp<T> cmp(beg);
      //std::sort(bkt.buf, bkt.buf+bkt.size, cmp);

      std::cerr << "CHECK[" << end-beg << "]" << std::endl;

      for(int i=0; i < bkt.size-1; i++) {
        const T* a = beg+bkt.buf[i];
        const T* b = beg+bkt.buf[i+1];
        for(;;a++, b++) {
          if(*a < *b)
            break;
          if(*a > *b) {
            std::cerr << i << "# not ordered!: "
                      << std::endl;
            if((end-beg) < 100) {
              std::cerr << "I :";
              for(int j=0; j < bkt.size; j++)
                printf("%02d ", j);
              std::cerr << std::endl;

              std::cerr << "S :";
              for(int j=0; j < bkt.size; j++)
                printf("%02d ", beg[j]);
              std::cerr << std::endl;
              
              std::cerr << "SA:";
              for(int j=0; j < bkt.size; j++)
                printf("%02d ", bkt.buf[j]);
              std::cerr << std::endl;
            }
            break;
          }
        }
      } 
      
      /*
      std::cerr << "S1: ";
      for(int i=0; i < s1.size(); i++)
        printf("%02d ", s1[i]);
      std::cerr << std::endl;
      */
      
      std::cerr << "DONE" << std::endl;
      return;
    } else {
      Buckets<unsigned> bkt2(s1.data(), s1.data()+s1.size());
      impl<unsigned>(s1.data(), s1.data()+s1.size(), bkt2); // XXX:

      // check
      /*
      std::cerr << "CHECK[" << end-beg << "]" << std::endl;
      for(int i=0; i < bkt.size-1; i++) {
        const T* a = beg+bkt.buf[i];
        const T* b = beg+bkt.buf[i+1];
        for(;;a++, b++) {
          if(*a < *b)
            break;
          if(*a > *b) {
            if((end-beg) < 100) {
              std::cerr << i << "# not ordered!" << std::endl;
              
              std::cerr << "I :";
              for(int j=0; j < bkt.size; j++)
                printf("%02d ", j);
              std::cerr << std::endl;

              std::cerr << "S :";
              for(int j=0; j < bkt.size; j++)
                printf("%02d ", beg[j]);
              std::cerr << std::endl;
              
              std::cerr << "SA:";
              for(int j=0; j < bkt.size; j++)
                printf("%02d ", bkt.buf[j]);
              std::cerr << std::endl;
            }
            break;
          }
        }
      } 
      */  
      //

      for(std::size_t i = 0; i < s1.size(); i++)
        s1[i] = lms_ary[bkt2.buf[i]];
      bkt.init();
      induce(beg, types, s1, bkt);
    }
  }

  template <typename T>
  void classify(const T* src, flags& types) {
    for(int i=(int)types.size()-2; i >= 0; i--) {
      if (src[i] < src[i+1])      ;
      else if (src[i] > src[i+1]) types[i] = 1;
      else                        types[i] = types[i+1];
    }
  } 

  template <typename T>
  void induce(const T* src, const flags& types, const lms_ary_t& lms_ary, Buckets<T>& bkt, bool o=false) {
    const unsigned len = (const unsigned)types.size();
    
    if(o) 
      for(std::size_t i=0; i < lms_ary.size(); i++)
        bkt.putS(src[lms_ary[i]], lms_ary[i]);
    else
      for(std::size_t i=lms_ary.size()-1; i != (std::size_t)-1; i--)
        bkt.putS(src[lms_ary[i]], lms_ary[i]);

    for(unsigned i=0; i < len; i++) {
      const int pos = bkt.position(i)-1;
      if(pos >= 0 && types[pos]==1) 
        bkt.putL(src[pos], pos);
    }

    bkt.initS();
        
    for(unsigned i=len-1; i > 0; i--) {
      const int pos = bkt.position(i)-1;
      if(pos >= 0 && types[pos]==0) 
        bkt.putS(src[pos], pos);
    }
  }

  template <typename T>
  bool reduce(const T* src, const flags& types, const Buckets<T>& bkt, unsigned lms_cnt, std::vector<unsigned>& s1) {
    const unsigned* sa = bkt.buf;
    const unsigned len = bkt.size;

    unsigned order = 0;
    s1[sa[0]/2] = order;

    unsigned prev = sa[0];
    for(unsigned i=1; i < len; i++) {
      const unsigned pos = sa[i];
      if(pos != 0 && isLMS(pos, types)) {
        if(lms_eql(src, types, prev, pos)==false)
          order++;
        if(s1[pos/2] != (unsigned)-1)
          std::cerr << "!!!: " << s1[pos] << std::endl;
        s1[pos/2] = order;
        prev = pos;
      }
    }

    std::cerr << "order: " << order << "#" << lms_cnt << std::endl;
    if(order == lms_cnt-1) {
      // XXX:
      s1.erase(std::remove(s1.begin(), s1.end(), -1), s1.end());
      return true; 
    }

    s1.erase(std::remove(s1.begin(), s1.end(), -1), s1.end());
    if(false) {
      for(int i=0; i < s1.size(); i++)
        std::cerr << "s1[" << i << "]: " << s1[i] << std::endl;
    }
    return false;
  }

  template <typename T>
  bool lms_eql(const T* src, const flags& types, unsigned i1, unsigned i2) const {
    if(src[i1] != src[i2])
      return false;
    if(types[i1] != types[i2])
      return false;
    
    for(i1++,i2++;; i1++, i2++) {
      if(src[i1] != src[i2])
        return false;
      if(types[i1] != types[i2])
        return false;
      if(isLMS(i1,types) && isLMS(i2,types))
        break;
    }
    return true;
  }
  
  void calc_lms_ary(const flags& types, std::vector<unsigned>& lms_ary) {
    // TODO: まとめられる
    for(std::size_t i = 1; i < types.size(); i++)
      if(isLMS(i,types))
        lms_ary.push_back(i);
  }

  bool isLMS(unsigned i, const flags& types) const {
    return types[i-1] > types[i];
  }
};

#endif
