#ifndef SA_IS_HH
#define SA_IS_HH

#include <vector>
#include <cstring>
#include <algorithm>

typedef std::vector<bool> flags;

struct BucketIndex {
    void set(unsigned start, unsigned last) {
        this->start = l_cur = start;
        this->last  = s_cur = last;
    }

    unsigned start;
    unsigned l_cur;
    unsigned s_cur;
    unsigned last;
};

template <typename T>
struct Buckets {
  Buckets(const T* beg, const T* end, unsigned* src_buf) : buf(src_buf) {
    unsigned limit = *std::max_element(beg, end)+1;
    idx.resize(limit);
    std::vector<unsigned> freq(limit, 0);
    
    for(const T* c=beg; c!=end; c++)
      ++freq[*c];

    unsigned offset=0;
    for(unsigned i=0; i < idx.size(); i++) {
        idx[i].set(offset, offset+freq[i]-1);
      offset += freq[i];
    }
        
    size = end-beg;
    memset(buf, 0xFF, size*sizeof(unsigned));
  }
  
  void initS() {
    for(unsigned i=0; i < idx.size(); i++)
        idx[i].s_cur = idx[i].last;
  }

  void init() {
      for(unsigned i=0; i < idx.size(); i++) 
          idx[i].set(idx[i].start, idx[i].last);
      memset(buf, 0xFF, size*sizeof(unsigned));
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
  std::vector<BucketIndex> idx;
  unsigned* buf;
  unsigned size;
};

typedef std::vector<unsigned> lms_ary_t;

class SA_IS {
private:
  const unsigned char* m_str;
  Buckets<unsigned char> m_bkt;
  
public:
  SA_IS(const char* str) 
    : m_str((const unsigned char*)str),
      m_bkt(m_str, m_str+strlen(str)+1, new unsigned[strlen(str)+1]) // TODO: delete
  {
      impl<unsigned char>(m_str, m_str+m_bkt.size, m_bkt);
  }

    ~SA_IS() {
        delete [] m_bkt.buf;
    }

  const unsigned* sa() const { return m_bkt.buf+1; }
  const unsigned size() const { return m_bkt.size-1; }

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
      for(std::size_t i = 0; i < s1.size(); i++)
        bkt.buf[s1[i]] = lms_ary[i];

      for(std::size_t i = 0; i < s1.size(); i++)
        s1[i] = bkt.buf[i];
    } else {
      Buckets<unsigned> bkt2(s1.data(), s1.data()+s1.size(), bkt.buf);
      impl<unsigned>(s1.data(), s1.data()+s1.size(), bkt2); // XXX: .data()はgcc依存

      for(std::size_t i = 0; i < s1.size(); i++)
        s1[i] = lms_ary[bkt2.buf[i]];
    }
    bkt.init();
    induce(beg, types, s1, bkt);
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
    for(std::size_t i=lms_ary.size()-1; i != (std::size_t)-1; i--)
      bkt.putS(src[lms_ary[i]], lms_ary[i]);

    for(unsigned i=0; i < types.size(); i++) {
      const int pos = bkt.position(i)-1;
      if(pos >= 0 && types[pos]==1) 
        bkt.putL(src[pos], pos);
    }

    bkt.initS();
        
    for(unsigned i=types.size()-1; i > 0; i--) {
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
    
    for(i1++,i2++;; i1++, i2++) {
      if(src[i1] != src[i2])                 return false;
      if(types[i1] != types[i2])             return false;
      if(isLMS(i1,types) && isLMS(i2,types)) return true;
    }
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
