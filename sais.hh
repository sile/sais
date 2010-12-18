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

struct Buckets {
  Buckets(const unsigned char* beg, const unsigned char* end) : buf(NULL) {
    // TODO: classify1とまとめられる
    unsigned freq[0x100] = {0};
    for(const unsigned char* c=beg; c!=end; c++)
      ++freq[(unsigned char)*c];
        
    unsigned offset=0;
    for(unsigned i=0; i < 0x100; i++) {
      idx[i].set(offset, offset+freq[i]-1);
      offset += freq[i];
    }
    idx[0x100].set(offset,offset);
        
    size = end-beg;
    buf = new unsigned[size];
    memset(buf,0xFF,size*sizeof(unsigned));
  }

  ~Buckets() {
    delete [] buf;
  }

  void initS() {
    for(unsigned i=0; i < 0x100; i++)
      idx[i].s_cur = idx[i+1].start-1;
  }

  void init() {
    for(unsigned i=0; i < 0x100; i++) 
      idx[i].set(idx[i].start,idx[i+1].start-1);
    memset(buf,0xFF,size*sizeof(unsigned));
  }

  void putL(char c, unsigned pos) {
    buf[idx[(unsigned char)c].l_cur++] = pos;
  }

  void putS(char c, unsigned pos) {
    buf[idx[(unsigned char)c].s_cur--] = pos;
  }
    
  // XXX:
  int position(unsigned i) const {
    return (int)buf[i];
  }

public:
  Index idx[0x101];
  unsigned* buf;
  unsigned size;
};

class SA_IS {
public:
  SA_IS(const char* text) : source((const unsigned char*)text) {
        
  }

  void construct() {
    impl1(source, source+strlen((const char*)source)+1);
  }

private:
  const unsigned char* source;
    
    
private:
  // TODO: template. byte-size
  void impl1(const unsigned char* beg, const unsigned char* end) {
    flags types(end-beg,0);
    classify1(beg, types);
    std::vector<unsigned> lms_ary;
    calc_lms_ary(types, lms_ary);
    Buckets bkt(beg,end);
    induceA(beg, types, lms_ary, bkt);

    std::vector<unsigned> s1(types.size()/2+1,(unsigned)-1);
    const bool uniq = reduce(beg, types, bkt, lms_ary.size(), s1);
    if(uniq) {
      for(int i=0; i < types.size(); i++) std::cout << bkt.buf[i] << " "; std::cout << std::endl;
      return;
    } else {
      // TODO: 一般化
      // impl(...)
      for(std::size_t i = s1.size()-1; i != (std::size_t)-1; i--) 
        s1[i] = lms_ary[s1[i]];
      bkt.init();
      induceA(beg, types, s1, bkt);
      // return bkt.buf
    }
  }

  void classify1(const unsigned char* src, flags& types) {
    for(int i=(int)types.size()-2; i >= 0; i--) {
      if (src[i] < src[i+1])      ;
      else if (src[i] > src[i+1]) types[i] = 1;
      else                        types[i] = types[i+1];
    }
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

  bool lms_eql(const unsigned char* src, const flags& types, unsigned i1, unsigned i2) const {
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

  bool reduce(const unsigned char* src, const flags& types, const Buckets& bkt, unsigned lms_cnt, std::vector<unsigned>& s1) {
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
        s1[pos/2] = order;
        prev = pos;
      }
    }

    std::cerr << "order: " << order << "#" << lms_cnt << std::endl;
    if(order < lms_cnt) {
      s1.erase(std::remove(s1.begin(), s1.end(), -1),s1.end());
      return false;
    }
    return true;
  }

  void induceA(const unsigned char* src, const flags& types, const std::vector<unsigned>& lms_ary, Buckets& bkt) {
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
};

#endif
