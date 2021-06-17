#ifndef HRTREE_ZIP_ZIP_HPP_INCLUDED
#define HRTREE_ZIP_ZIP_HPP_INCLUDED

#include <cstddef>
#include <type_traits>
#include <iterator>


namespace hrtree {

  namespace zip {

    namespace detail {

      template <typename IT1, typename... Tail>
      struct least_iter_cat_impl
      {
        typedef typename std::iterator_traits<IT1>::iterator_category CAT1;
        typedef typename least_iter_cat_impl<Tail...>::type CAT2;
        typedef typename std::conditional <
          std::is_base_of<CAT1, CAT2>::value,
          CAT1,
          CAT2
        > ::type type;
      };


      template <typename IT>
      struct least_iter_cat_impl < IT, void >
      {
        typedef typename std::iterator_traits<IT>::iterator_category type;
      };


      // Finds the least iterator category in a list of iterators.
      template <typename... ITs>
      struct least_iter_cat
      {
        typedef typename least_iter_cat_impl<ITs..., void>::type type;
      };


      template <int i, int... tail>
      struct index_sequence : index_sequence < i - 1, i, tail... >
      {
      };


      template <typename... T>
      inline void consume_comma_sep(T&...)
      {
      }

      
      template <int... idx>
      struct index_sequence < -1, idx... >
      {
        template <typename ZIT>
        static auto deref(typename ZIT::type const& zit) -> typename ZIT::reference
        {
          return ZIT::reference(*std::get<idx>(zit)...);
        }

        template <typename ZIT>
        static auto ptr(typename ZIT::type& zit) -> typename ZIT::pointer
        {
          return ZIT::reference(std::addressof(*std::get<idx>(zit))...);
        }

        template <typename ZIT>
        static void preincr(ZIT& zit)
        {
          consume_comma_sep(++std::get<idx>(zit)...);
        }

        template <typename ZIT>
        static void predecr(ZIT& zit)
        {
          consume_comma_sep(--std::get<idx>(zit)...);
        }

        template <typename ZIT>
        static void advance(ZIT& zit, ptrdiff_t i)
        {
          consume_comma_sep(std::get<idx>(zit) += i...);
        }
      };


    }


    template <typename Head, typename... Tail>
    class zip
    {
      typedef detail::index_sequence<sizeof...(Tail)> indexed;

    public:
      typedef std::tuple<Head, Tail...> type;
      typedef typename detail::least_iter_cat<Head, Tail...>::type iterator_category;
      typedef typename std::tuple<typename std::iterator_traits<Head>::pointer, typename std::iterator_traits<Tail>::pointer...>  pointer;
      typedef typename std::tuple<typename std::iterator_traits<Head>::reference, typename std::iterator_traits<Tail>::reference...>  reference;
      typedef typename std::tuple<typename std::iterator_traits<Head>::value_type, typename std::iterator_traits<Tail>::value_type...>  value_type;
      typedef ptrdiff_t difference_type;
      typedef ptrdiff_t distance_type;

      zip() {}
      explicit zip(Head head, Tail... tail) : self_(head, tail...) {}

      Head head() const { return std::get<0>(self_); }
      
      template <int I>
      auto get() -> std::tuple_element_t<I, type> const { return std::get<I>(self_); }

      reference operator * () const { return indexed::template deref<zip>(self_); }
      pointer operator -> () const { return indexed::template ptr<zip>(self_); }
      reference operator [] (size_t i) const { return *(*this + i); }

      zip& operator ++ () { indexed::template preincr(self_); return *this; }
      zip& operator -- () { indexed::template predecr(self_); return *this; }
      zip operator ++ (int) { zip tmp(*this); return ++tmp; }
      zip operator -- (int) { zip tmp(*this); return ++tmp; }
      zip& operator += (ptrdiff_t n) { indexed::template advance(self_, +n); return *this; }
      zip& operator -= (ptrdiff_t n) { indexed:: template advance(self_, -n); return *this; }

      bool operator == (const zip& x) const { return self_ == x.self_; }
      bool operator != (const zip& x) const { return self_ != x.self_; }
      bool operator < (const zip& x) const { return self_ <= x.self_; }

      friend zip operator + (zip x, ptrdiff_t n) { return x += n; }
      friend zip operator + (ptrdiff_t n, zip x) { return x += n; }
      friend zip operator - (zip x, ptrdiff_t n) { return x -= n; }
      friend ptrdiff_t operator - (const zip& a, const zip& b) { return a.head() - b.head(); }

    private:
      type self_;
    };


    template <typename... ITs>
    inline zip<ITs...> make_zip(ITs&&... its)
    {
      return zip<ITs...>(std::forward<ITs>(its)...);
    }


    template <typename Head, typename... Tail>
    Head head(zip<Head, Tail...> const& zip)
    {
      return zip.head();
    }


    template <typename Head, typename... Tail>
    Head head(std::tuple<Head, Tail...> const& tup)
    {
      return std::get<0>(tup);
    }


    template <typename IT>
    IT head(IT it)
    {
      return it;
    }


    template <typename IT>
    struct head_type
    {
      typedef IT type;
    };


    template <typename Head, typename... Tail>
    struct head_type<zip<Head, Tail...>>
    {
      typedef Head type;
    };


    namespace detail {

      template <int I>
      struct iter_move0 
      {
        template <typename ZIT>
        static void apply(ZIT& src, ZIT& dst, ptrdiff_t srcIdx, ptrdiff_t dstIdx)
        {
          iter_move0<I-1>::template apply(src, dst, srcIdx, dstIdx);
          *(dst.template get<I>() + dstIdx) = std::move(*(src.template get<I>() + srcIdx));
        }
      };


      template <>
      struct iter_move0<-1>
      {
        template <typename ZIT>
        static void apply(ZIT&, ZIT &, ptrdiff_t, ptrdiff_t) {}
      };

    }


    template <typename Head, typename... Tail>
    inline void iter_move(zip<Head, Tail...> & src, zip<Head, Tail...> & dst, ptrdiff_t srcIdx, ptrdiff_t dstIdx)
    {
      detail::iter_move0<sizeof...(Tail)>::template apply(src, dst, srcIdx, dstIdx);
    }


    template <typename IT>
    inline void iter_move(IT src, IT dst, ptrdiff_t srcIdx, ptrdiff_t dstIdx)
    {
      *(dst + dstIdx) = std::move(*(src + srcIdx));
    }
  }

}

#endif
