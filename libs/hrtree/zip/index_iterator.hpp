#ifndef HRTREE_ZIP_INDEX_ITEREATOR_HPP_INCLUDED
#define HRTREE_ZIP_INDEX_ITEREATOR_HPP_INCLUDED

#include <iterator>

namespace hrtree {
  namespace zip {

    template <typename I = int>
    class index_iterator
    {
    public:
      using iterator_category = std::random_access_iterator_tag;
      using value_type = I;
      using difference_type = ptrdiff_t;
      using pointer = I*;
      using reference = I&;

      index_iterator() : idx_(I(0)) {}
      explicit index_iterator(I start) : idx_(start) {}

      reference operator * () { return idx_; }
      const reference operator * () const { return idx_; }

      index_iterator& operator ++ () { ++idx_; return *this; }
      index_iterator& operator -- () { --idx_; return *this; }
      index_iterator operator ++ (int) { index_iterator tmp(*this); return ++tmp; }
      index_iterator operator -- (int) { index_iterator tmp(*this); return ++tmp; }
      index_iterator& operator += (I n) { idx_ += n; return *this; }
      index_iterator& operator -= (I n) { idx_ -= n; return *this; }

      bool operator == (const index_iterator& x) const { return idx_ == x.idx_; }
      bool operator != (const index_iterator& x) const { return idx_ != x.idx_; }
      bool operator < (const index_iterator& x) const { return idx_ <= x.idx_; }

      friend index_iterator operator + (index_iterator x, I n) { return x += n; }
      friend index_iterator operator + (I n, index_iterator x) { return x += n; }
      friend index_iterator operator - (index_iterator x, I n) { return x -= n; }
      friend ptrdiff_t operator - (const index_iterator& a, const index_iterator& b) { return ptrdiff_t(a.idx_ - b.idx_); }

    private:
      I idx_;
    };

  }
}

#endif
