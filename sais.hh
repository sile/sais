#ifndef SA_IS_HH
#define SA_IS_HH

#include <vector>
#include <cstring>
#include <algorithm>

typedef std::vector<bool> flags;

struct BucketIndex {
  void set(unsigned start, unsigned last) {
    this->start = l_cur = start;
    s_cur = last;
  }

  unsigned start;
  unsigned l_cur;
  unsigned s_cur;
};

template <typename T>
struct Buckets {
  Buckets(const T* beg, const T* end, unsigned* src_buf) : buf(src_buf) {
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
    memset(buf, 0xFF, size*sizeof(unsigned));
  }
  
  void initS() {
    for(unsigned i=0; i < idx.size()-1; i++)
      idx[i].s_cur = idx[i+1].start-1;
  }

  void init() {
    for(unsigned i=0; i < idx.size()-1; i++) 
      idx[i].set(idx[i].start, idx[i+1].start-1);
  }

  void putL(T c, unsigned pos) {
    buf[idx[c].l_cur++] = pos;
  }

  void putS(T c, unsigned pos) {
    buf[idx[c].s_cur--] = pos;
  }
    
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
    const unsigned len = end-beg;
    flags types(end-beg, 0);
    classify(beg, types);
    induce(beg, types, bkt);

    std::cerr << 1 << "#" << (end-beg) << std::endl;

    unsigned *s1;
    unsigned s1_len;
    const bool uniq = reduce(beg, types, bkt, s1, s1_len);
    if(!uniq) {
      Buckets<unsigned> bkt2(s1, s1+s1_len, bkt.buf);
      impl<unsigned>(s1, s1+s1_len, bkt2);

      for(std::size_t i = 0; i < s1_len; i++)
        s1[bkt2.buf[i]] = i;
    }

    for(std::size_t i = 1, j=0; i < len; i++)
      if(isLMS(i,types))
        bkt.buf[s1[j++]] = -i;

    memset(bkt.buf+s1_len, 0, (bkt.size-s1_len)*sizeof(unsigned));

    bkt.init(); 
    induce(beg, types, bkt.buf, s1_len, bkt);
  }

  template <typename T>
  void classify(const T* src, flags& types) {
    for(int i=(int)types.size()-2; i >= 0; i--)
      if (src[i] < src[i+1])      ;
      else if (src[i] > src[i+1]) types[i] = 1;
      else                        types[i] = types[i+1];
  } 

  template <typename T>
  void induce(const T* src, const flags& types, const unsigned* lms_ary, unsigned lms_len, Buckets<T>& bkt) {
    for(std::size_t i=lms_len-1; i != (std::size_t)-1; i--)
      bkt.putS(src[-lms_ary[i]], -lms_ary[i]);

    induceLS(src,types,bkt);
  }

  template <typename T>
  void induce(const T* src, const flags& types, Buckets<T>& bkt) {
    for(unsigned i=1; i < types.size(); i++)
      if(isLMS(i,types))
        bkt.putS(src[i], i);
    
    induceLS(src,types,bkt);
  }

  template <typename T>
  void induceLS(const T* src, const flags& types, Buckets<T>& bkt) {
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
  bool reduce(const T* src, const flags& types, const Buckets<T>& bkt, 
              unsigned *& s1, unsigned& s1_len) {
    unsigned *sa = bkt.buf;
    s1 = bkt.buf + bkt.size/2;
    s1_len = bkt.size - bkt.size/2;

    unsigned sa_len=0;
    sa[sa_len++] = bkt.buf[0];
    for(unsigned i=1; i < bkt.size; i++) 
      if(bkt.buf[i] != 0 && isLMS(bkt.buf[i], types))
        sa[sa_len++] = bkt.buf[i];
    memset(s1,0xFF,s1_len*sizeof(unsigned));

    unsigned order = 0;
      
    s1[(sa[0]-1)/2] = order;
      
    unsigned prev = sa[0];
    for(unsigned i=1; i < sa_len; i++) {
      const unsigned pos = sa[i];

      if(lms_eql(src, types, prev, pos)==false)
        order++;
      s1[(pos-1)/2] = order;
      prev = pos;
    }

    s1_len = std::remove(s1, s1+s1_len, -1) - s1;

    return order+1 == s1_len;
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

  bool isLMS(unsigned i, const flags& types) const {
    return types[i-1] > types[i];
  }
};

#endif
