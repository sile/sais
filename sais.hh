#ifndef SAIS_HH
#define SAIS_HH

#include <vector>
#include <cstring>
#include <algorithm>

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
  Buckets(const T* beg, const T* end, int* buf) 
    : size(end-beg), buf(buf) {
    const int limit = *std::max_element(beg, end)+1;
    idx.resize(limit+1);
    
    std::vector<unsigned> freq(limit, 0);
    for(const T* c=beg; c!=end; c++)
      ++freq[*c];

    unsigned offset=0;
    for(unsigned i=0; i < idx.size(); i++) {
      idx[i].set(offset, offset+freq[i]-1);
      offset += freq[i];
    }
    idx[limit].set(offset, offset);
        
    memset(buf, 0, size*sizeof(int));
  }
  
  void initS() {
    for(unsigned i=0; i < idx.size()-1; i++)
      idx[i].s_cur = idx[i+1].start-1;
  }

  void init() {
    for(unsigned i=0; i < idx.size()-1; i++) 
      idx[i].set(idx[i].start, idx[i+1].start-1);
  }

  void putL(T c, int pos) { buf[idx[c].l_cur++] = pos; }
  void putS(T c, int pos) { buf[idx[c].s_cur--] = pos; }

  std::vector<BucketIndex> idx;
  const unsigned size;
  int* buf;
};

class SAIS {
  typedef std::vector<bool> SLtypes;
  
public:
  SAIS(const char* str) : m_buf_len(strlen(str)+1), m_bucket_buf(new int[m_buf_len]) {
    const unsigned char* s = reinterpret_cast<const unsigned char*>(str);
    impl<unsigned char>(s, m_buf_len, m_bucket_buf);
  }

  ~SAIS() {
    delete [] m_bucket_buf;
  }

  const int* sa() const { return m_bucket_buf+1; }
  const unsigned size() const { return m_buf_len-1; }

private:
  template <typename T>
  void impl(const T* str, const unsigned length, int* bucket_buf) {
    Buckets<T> bkt(str, str+length, bucket_buf);
    SLtypes types(length);
    classify(str, types);
    induce(str, types, bkt);

    int *s1;
    unsigned s1_len;
    const bool uniq = reduce(str, types, bkt, s1, s1_len);
    
    if(!uniq) {
      impl<int>(s1, s1_len, bkt.buf);
      for(std::size_t i = 0; i < s1_len; i++)
        s1[bkt.buf[i]] = i;
    }

    for(std::size_t i = 1, j=0; i < length; i++)
      if(isLMS(i,types))
        bkt.buf[s1[j++]] = -i;
    memset(bkt.buf+s1_len, 0, (bkt.size-s1_len)*sizeof(int));

    bkt.init(); 
    induce(str, types, s1_len, bkt);
  }

  template <typename T>
  void classify(const T* str, SLtypes& types) const {
    for(int i=static_cast<int>(types.size())-2; i >= 0; i--)
      if (str[i] < str[i+1])      types[i] = 0;
      else if (str[i] > str[i+1]) types[i] = 1;
      else                        types[i] = types[i+1];
  } 

  template <typename T>
  void induce(const T* str, const SLtypes& types, unsigned lms_ary_len, Buckets<T>& bkt) {
    const int* ordered_lms = bkt.buf;
    for(int i = static_cast<int>(lms_ary_len)-1; i >= 0; i--)
      bkt.putS(str[-ordered_lms[i]], -ordered_lms[i]);

    induceLS(str, types, bkt);
  }

  template <typename T>
  void induce(const T* str, const SLtypes& types, Buckets<T>& bkt) {
    for(std::size_t i=1; i < types.size(); i++)
      if(isLMS(i,types))
        bkt.putS(str[i], i);
    
    induceLS(str,types,bkt);
  }

  template <typename T>
  void induceLS(const T* str, const SLtypes& types, Buckets<T>& bkt) {
    for(std::size_t i=0; i < types.size(); i++) {
      const int pos = bkt.buf[i]-1;
      if(pos >= 0 && types[pos]==1) 
        bkt.putL(str[pos], pos);
    }

    bkt.initS();
    for(std::size_t i=types.size()-1; i > 0; i--) {
      const int pos = bkt.buf[i]-1;
      if(pos >= 0 && types[pos]==0) 
        bkt.putS(str[pos], pos);
    }
  }

  template <typename T>
  bool reduce(const T* str, const SLtypes& types, const Buckets<T>& bkt, int *& s1, unsigned& s1_len) {
    int *sa = bkt.buf;
    s1 = bkt.buf + bkt.size/2;
    s1_len = bkt.size - bkt.size/2;

    unsigned sa_len=0;
    for(unsigned i=0; i < bkt.size; i++) 
      if(bkt.buf[i] != 0 && isLMS(bkt.buf[i], types))
        sa[sa_len++] = bkt.buf[i];
    memset(s1, -1, s1_len*sizeof(int));

    unsigned order = 0;
    s1[(sa[0]-1)/2] = order;
    for(unsigned i=1; i < sa_len; i++)
      s1[(sa[i]-1)/2] = lms_eql(str,types,sa[i-1],sa[i]) ? order : ++order;

    s1_len = std::remove(s1, s1+s1_len, -1) - s1;
    return order+1 == s1_len;
  }

  template <typename T>
  bool lms_eql(const T* str, const SLtypes& types, unsigned i1, unsigned i2) const {
    for(;; i1++,i2++) 
      if(str[i1] != str[i2] || types[i1] != types[i2]) return false;
      else if(isLMS(i1+1,types) && isLMS(i2+1,types))  return str[i1+1] == str[i2+1];
  }

  bool isLMS(unsigned i, const SLtypes& types) const {
    return types[i-1] > types[i];
  }

private:
  const unsigned m_buf_len;
  int* m_bucket_buf;
};

#endif
