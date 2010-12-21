#ifndef SA_IS_HH
#define SA_IS_HH

#include <vector>
#include <cstring>
#include <algorithm>

typedef std::vector<bool> flags;

class BucketIndex {
public:
    BucketIndex(unsigned start, unsigned last) 
      : start(start), l_cur(start), last(last), s_cur(last) {}
  
  BucketIndex() {}


    void reset_Spos() {
        s_cur = last;
    }

    void reset() {
        l_cur = start;
        s_cur = last;
    }
    
    unsigned next_Spos() { return s_cur--; }
    unsigned next_Lpos() { return l_cur++; }
    
private:
    unsigned start;
    unsigned l_cur;
    unsigned s_cur;
    unsigned last;
};

template <typename T>
struct Buckets {
  Buckets(const T* beg, const T* end) {
    unsigned limit = *std::max_element(beg, end)+1;
    idx.resize(limit);
    std::vector<unsigned> freq(limit, 0);
    
    for(const T* c=beg; c!=end; c++)
      ++freq[*c];

    unsigned offset=0;
    for(unsigned i=0; i < idx.size(); i++) {
      idx[i] = BucketIndex(offset, offset+freq[i]-1);
      offset += freq[i];
    }
        
    buf.resize(end-beg, (unsigned)-1);
  }

  void initS() {
    for(unsigned i=0; i < idx.size(); i++)
      idx[i].reset_Spos();
  }

  void init() {
    for(unsigned i=0; i < idx.size(); i++) 
      idx[i].reset();
    std::fill(buf.begin(), buf.end(), (unsigned)-1);
  }

  void putL(T c, unsigned pos) {
    buf[idx[c].next_Lpos()] = pos;
  }

  void putS(T c, unsigned pos) {
    buf[idx[c].next_Spos()] = pos;
  }
    
  // XXX:
  int position(unsigned i) const {
    return (int)buf[i];
  }

public:
  std::vector<BucketIndex> idx;
  std::vector<unsigned> buf;
};

typedef std::vector<unsigned> lms_ary_t;

class SA_IS {
private:
  const unsigned char* m_str;
  Buckets<unsigned char> m_bkt;
  
public:
  SA_IS(const char* str) 
    : m_str((const unsigned char*)str),
      m_bkt(m_str, m_str+strlen(str)+1)
  {}

  void construct() {
    impl<unsigned char>(m_str, m_str+m_bkt.buf.size(), m_bkt);
  }

private:
  template <typename T>
  void impl(const T* beg, const T* end, Buckets<T>& bkt) {
    flags types(end-beg, 0);
    lms_ary_t lms_ary;
    classify(beg, types);
    calc_lms_ary(types, lms_ary);
    induce(beg, types, lms_ary, bkt);

    std::vector<unsigned> s1(types.size()/2+1,(unsigned)-1);
    const bool uniq = reduce(beg, types, bkt, lms_ary.size(), s1);
    if(uniq) {
      std::vector<unsigned> s2(s1.size()); // XXX: inplaceで可能
      for(std::size_t i = 0; i < s1.size(); i++)
          s2[s1[i]] = lms_ary[i];
      
      bkt.init();
      induce(beg, types, s2, bkt);
    } else {
      Buckets<unsigned> bkt2(s1.data(), s1.data()+s1.size()); // bktとメモリ領域は共有可能
      impl<unsigned>(s1.data(), s1.data()+s1.size(), bkt2); // XXX:

      for(std::size_t i = 0; i < s1.size(); i++)
        s1[i] = lms_ary[bkt2.buf[i]];
      bkt.init();
      induce(beg, types, s1, bkt);
    }
  }

  template <typename T>
  void classify(const T* src, flags& types) {
    for(int i=(int)types.size()-2; i >= 0; i--)
      if (src[i] < src[i+1])      ;
      else if (src[i] > src[i+1]) types[i] = 1;
      else                        types[i] = types[i+1];
  } 

  template <typename T>
  void induce(const T* src, const flags& types, const lms_ary_t& lms_ary, Buckets<T>& bkt) {
    const unsigned len = (const unsigned)types.size();
    
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
    const std::vector<unsigned>& sa = bkt.buf;
    const unsigned len = sa.size();

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

    s1.erase(std::remove(s1.begin(), s1.end(), -1), s1.end());
    return order==lms_cnt-1;
  }

  template <typename T>
  bool lms_eql(const T* src, const flags& types, unsigned i1, unsigned i2) const {
    if(src[i1] != src[i2])     return false;
    if(types[i1] != types[i2]) return false;
    
    for(i1++,i2++;; i1++, i2++)
      if(src[i1] != src[i2])                 return false;
      if(types[i1] != types[i2])             return false;
      if(isLMS(i1,types) && isLMS(i2,types)) return true;
  }
  
  void calc_lms_ary(const flags& types, std::vector<unsigned>& lms_ary) {
    for(std::size_t i = 1; i < types.size(); i++)
      if(isLMS(i,types))
        lms_ary.push_back(i);
  }

  bool isLMS(unsigned i, const flags& types) const {
    return types[i-1] > types[i];
  }
};

#endif
