#ifndef SA_IS_HH
#define SA_IS_HH

#include <vector>
#include <cstring>
#include <algorithm>

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
struct Buckets2 {
  Buckets2(const T* beg, const T* end) : buf(NULL) {
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

  ~Buckets2() {
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
    impl<unsigned char>(source, source+strlen((const char*)source)+1);
  }

private:
  const unsigned char* source;
    
    
private:
  template <typename T>
  void impl(const T* beg, const T* end) {
    flags types(end-beg, 0);
    classify(beg, types);
    lms_ary_t lms_ary;
    calc_lms_ary(types, lms_ary);
    Buckets2<T> bkt(beg,end);// XXX:
    induce(beg, types, lms_ary, bkt);

    std::vector<unsigned> s1(types.size()/2+1,(unsigned)-1);
    const bool uniq = reduceT(beg, types, bkt, lms_ary.size(), s1);
    if(uniq) {
      //for(int i=0; i < types.size(); i++) std::cout << bkt.buf[i] << " "; std::cout << std::endl;
      std::cout << "DONE" << std::endl;
      return;
    } else {
      impl<unsigned>(s1.data(), s1.data()+s1.size()); // XXX:
      for(std::size_t i = s1.size()-1; i != (std::size_t)-1; i--) 
        s1[i] = lms_ary[s1[i]];
      bkt.init();
      induce(beg, types, s1, bkt);
    }
  }

  template <typename T>
  void classify(const T src, flags& types) {
    for(int i=(int)types.size()-2; i >= 0; i--) {
      if (src[i] < src[i+1])      ;
      else if (src[i] > src[i+1]) types[i] = 1;
      else                        types[i] = types[i+1];
    }
  } 

  template <typename T, typename B>
  void induce(const T src, const flags& types, const lms_ary_t& lms_ary, Buckets2<B>& bkt) {
    const unsigned len = (const unsigned)types.size();
    
    // TODO: 逆順
    for(std::size_t i=0; i < lms_ary.size(); i++)
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

  template <typename T, typename B>
  bool reduceT(const T src, const flags& types, const Buckets2<B>& bkt, unsigned lms_cnt, std::vector<unsigned>& s1) {
    const unsigned* sa = bkt.buf;
    const unsigned len = bkt.size;

    unsigned order = 0;
    s1[sa[0]/2] = order;

    unsigned prev = sa[0];
    for(unsigned i=1; i < len; i++) {
      const unsigned pos = sa[i];
      if(pos != 0 && isLMS(pos, types)) {
        if(lms_eqlT(src, types, prev, pos)==false)
          order++;
        s1[pos/2] = order;
        prev = pos;
      }
    }

    std::cerr << "order: " << order << "#" << lms_cnt << std::endl;
    if(order+1 < lms_cnt) {
      s1.erase(std::remove(s1.begin(), s1.end(), -1),s1.end());
      return false;
    }
    return true;
  }

  template <typename T>
  bool lms_eqlT(const T src, const flags& types, unsigned i1, unsigned i2) const {
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
