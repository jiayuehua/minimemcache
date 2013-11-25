////////////////////////////////////////////////////////////////////////////////
// flex_string
// Copyright (c) 2001 by Andrei Alexandrescu
// Permission to use, copy, modify, distribute and sell this software for any
//     purpose is hereby granted without fee, provided that the above copyright
//     notice appear in all copies and that both that copyright notice and this
//     permission notice appear in supporting documentation.
// The author makes no representations about the
//     suitability of this software for any purpose. It is provided "as is"
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 by Jiayuehua 
////////////////////////////////////////////////////////////////////////////////
#ifndef FLEX_STRING_SHELL_INC_
#define FLEX_STRING_SHELL_INC_

// $Id: flex_string_shell.h 948 2009-01-26 01:55:50Z rich_sposato $


///////////////////////////////////////////////////////////////////////////////
// class template flex_string
// This file does not include any storage policy headers
///////////////////////////////////////////////////////////////////////////////

#include <istream>
#include <memory>
#include <algorithm>
#include <functional>
#include <cassert>
#include <limits>
#include <stdexcept>
#include "flex_string_details.h"
//#include <string>
#include <iosfwd>
namespace yasli{
// Forward declaration for default storage policy
template <typename E, class A> class AllocatorStringStorage;


template <class T> class mallocator
{
public:
    typedef T                 value_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef value_type&       reference;
    typedef const value_type& const_reference;
    typedef std::size_t       size_type;
    //typedef unsigned int      size_type;
    //typedef std::ptrdiff_t    difference_type;
    typedef int               difference_type;

    template <class U> 
    struct rebind { typedef mallocator<U> other; };

    mallocator() {}
    mallocator(const mallocator&) {}
    //template <class U> 
    //mallocator(const mallocator<U>&) {}
    ~mallocator() {}

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const 
    { 
        return x;
    }

    pointer allocate(size_type n, const_pointer = 0) 
    {
        using namespace std;
        void* p = malloc(n * sizeof(T));
        if (!p) throw bad_alloc();
        return static_cast<pointer>(p);
    }

    void deallocate(pointer p, size_type) 
    { 
        using namespace std;
        free(p); 
    }

    size_type max_size() const 
    { 
        return static_cast<size_type>(-1) / sizeof(T);
    }

    void construct(pointer p, const value_type& x) 
    { 
        new(p) value_type(x); 
    }

    void destroy(pointer p) 
    { 
        p->~value_type(); 
    }

private:
    void operator=(const mallocator&);
};

template<> class mallocator<void>
{
  typedef void        value_type;
  typedef void*       pointer;
  typedef const void* const_pointer;

  template <class U> 
  struct rebind { typedef mallocator<U> other; };
};

template <class T>
inline bool operator==(const mallocator<T>&, 
                       const mallocator<T>&) {
  return true;
}

template <class T>
inline bool operator!=(const mallocator<T>&, 
                       const mallocator<T>&) {
  return false;
}

template <class Allocator>
typename Allocator::pointer Reallocate(
    Allocator& alloc,
    typename Allocator::pointer p, 
    typename Allocator::size_type oldObjCount,
    typename Allocator::size_type newObjCount,
    void*)
{
    // @@@ not implemented
}

template <class Allocator>
typename Allocator::pointer Reallocate(
    Allocator& alloc,
    typename Allocator::pointer p, 
    typename Allocator::size_type oldObjCount,
    typename Allocator::size_type newObjCount,
    mallocator<void>*)
{
    // @@@ not implemented
}

template <typename E1, class T1, class A1, class S1>
struct Invariant;
template <class size_type>
    size_type Min(size_type lhs, size_type rhs)
    { return lhs < rhs ? lhs : rhs; }
template <class size_type>
    size_type Max(size_type lhs, size_type rhs)
    { return lhs > rhs ? lhs : rhs; }
////////////////////////////////////////////////////////////////////////////////
// class template flex_string
// a std::basic_string compatible implementation 
// Uses a Storage policy 
////////////////////////////////////////////////////////////////////////////////
template <typename E, class T , class A , class Storage >
class flex_string ;
template <typename E, class T, class A, class S>
void resize(flex_string<E, T, A, S>& s, typename flex_string<E, T, A, S>::size_type n, typename flex_string<E, T, A, S>::value_type c);
template <typename E, class T, class A, class S, class InputIterator>
flex_string<E, T, A, S>& append(flex_string<E, T, A, S>& s1, InputIterator first, InputIterator last);
template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& append(flex_string<E, T, A, S>& s1, const flex_string<E, T, A, S>& str, const typename A::size_type pos, typename A::size_type n);
template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& append(flex_string<E, T, A, S>& s1, typename flex_string<E, T, A, S>::size_type n, typename flex_string<E, T, A, S>::value_type c);
template< class size_type>
void Procust(size_type& n, size_type nmax)
{ if (n > nmax) n = nmax; }
template <typename Exception>
void Enforce(bool condition, Exception*, const char* msg)
{ if (!condition) throw Exception(msg); }
template <typename E,
    class T = std::char_traits<E>,
    class A = std::allocator<E>,
    class Storage = AllocatorStringStorage<E, A> >
class flex_string : private Storage
{
public:
    // types
    typedef T traits_type;
    typedef typename traits_type::char_type value_type;
    typedef A allocator_type;
    typedef typename A::size_type size_type;
    typedef typename A::difference_type difference_type;
    
    typedef typename Storage::reference reference;
    typedef typename A::const_reference const_reference;
    typedef typename A::pointer pointer;
    typedef typename A::const_pointer const_pointer;
    
    typedef typename Storage::iterator iterator;
    typedef typename Storage::const_iterator const_iterator;
    typedef std::reverse_iterator<iterator
#ifdef NO_ITERATOR_TRAITS
    , value_type
#endif
    > reverse_iterator;
    typedef std::reverse_iterator<const_iterator
#ifdef NO_ITERATOR_TRAITS
    , const value_type 
#endif
    > const_reverse_iterator;

    static const size_type npos;    // = size_type(-1)

private:

    flex_string& append_impl(const value_type* s, const size_type n)
    { 
        Invariant<E, T, A, Storage> checker(*this); 
        (void) checker; 
        static std::less_equal<const value_type*> le;
        if (le(&*begin(), s) && le(s, &*end())) // aliasing
        {
            const size_type offset = s - &*begin();
            Storage::reserve(size() + n);
            s = &*begin() + offset;
        }
        Storage::append(s, s + n); 
        return *this;
    }
    
    void resize_impl(size_type n, value_type c)
    { Storage::resize(n, c); }
    
public:    
    // 21.3.1 construct/copy/destroy
    explicit flex_string(const A& a = A())
    : Storage(a) 
    {}
    
    flex_string(const flex_string& str)
    : Storage(str) 
    {}
    
    flex_string(const flex_string& str, size_type pos, 
        size_type n = npos, const A& a = A())
    : Storage(a) 
    {
        assign(*this, str, pos, n);
    }
    
    flex_string(const value_type* s, const A& a = A())
    : Storage(s, traits_type::length(s), a)
    {}
    
    flex_string(const value_type* s, size_type n, const A& a = A())
    : Storage(s, n, a)
    {}
    
    flex_string(size_type n, value_type c, const A& a = A())
    : Storage(n, c, a)
    {}

    template <class InputIterator>
    flex_string(InputIterator begin, InputIterator end, const A& a = A())
    : Storage(a)
    {
        assign(*this, begin, end);
    }

    ~flex_string()
    {}
    
    flex_string& operator=(const flex_string& str)
    {
        Storage& s = *this;
        s = str;
        return *this;
    }   
    
    flex_string& operator=(const value_type* s)
    {
        assign(*this, s);
        return *this;
    }

    flex_string& operator=(value_type c)
    {
        assign(*this, 1, c);
        return *this;
    }

    const_reference operator[](size_type pos) const
    { return *(c_str() + pos); }
    
    reference operator[](size_type pos)
    { return *(begin() + pos); }

    
    // 21.3.2 iterators:
    iterator begin()
    { return Storage::begin(); }
    
    const_iterator begin() const
    { return Storage::begin(); }
    
    iterator end()
    { return Storage::end(); }
    
    const_iterator end() const
    { return Storage::end(); }

    reverse_iterator rbegin()
    { return reverse_iterator(end()); }
    
    const_reverse_iterator rbegin() const
    { return const_reverse_iterator(end()); }
    
    reverse_iterator rend()
    { return reverse_iterator(begin()); }
    
    const_reverse_iterator rend() const
    { return const_reverse_iterator(begin()); }
    
    // 21.3.3 capacity:
    size_type size() const
    { return Storage::size(); }
    
    size_type max_size() const
    { return Storage::max_size(); }

    void resize(size_type n, value_type c)
    { Storage::resize(n, c); }

    size_type capacity() const
    { return Storage::capacity(); }
    
    void reserve(size_type res_arg = 0)
    {
        Enforce(res_arg <= max_size(), static_cast<std::length_error*>(0), "");
        Storage::reserve(res_arg);
    }
    
    // 21.3.5 modifiers:
	/*template <typename E1, class T1, class A1, class S1>
	friend flex_string& append(flex_string<E1, T1, A1, S1>&s1, const typename flex_string<E1, T1, A1, S1>::value_type* s, const typename flex_string<E1, T1, A1, S1>::size_type n);*/
	flex_string& append(const value_type* s, const  size_type n)
	{
		append_impl(s,n);
		return *this;
	}
    
private:
    template <int i> class Selector {};

    flex_string& InsertImplDiscr(iterator p, 
        size_type n, value_type c, Selector<1>)
    { 
        Invariant<E, T, A, Storage> checker(*this); 
        (void) checker; 
        assert(p >= begin() && p <= end());
        if (capacity() - size() < n)
        {
            const size_type sz = p - begin();
            reserve(size() + n);
            p = begin() + sz;
        }
        const iterator oldEnd = end();
        //if (p + n < oldEnd) // replaced because of crash (pk)
        if( n < size_type(oldEnd - p))
        {
			yasli::append(*this, oldEnd - n, oldEnd);
            //std::copy(
            //    reverse_iterator(oldEnd - n), 
            //    reverse_iterator(p), 
            //    reverse_iterator(oldEnd));
            flex_string_details::pod_move(&*p, &*oldEnd - n, &*p + n);
            std::fill(p, p + n, c);
        }
        else
        {
			yasli::append(*this, n - (end() - p), c);
			yasli::append(*this, p, oldEnd);
            std::fill(p, oldEnd, c);
        }
        return *this;
    }    

    template<class InputIterator>
    flex_string& InsertImplDiscr(iterator i,
        InputIterator b, InputIterator e, Selector<0>)
    { 
        InsertImpl(i, b, e, 
            typename std::iterator_traits<InputIterator>::iterator_category());
        return *this;
    }

    template <class FwdIterator>
    void InsertImpl(iterator i,
        FwdIterator s1, FwdIterator s2, std::forward_iterator_tag)
    { 
        Invariant<E, T, A, Storage>  checker(*this); 
        (void) checker;
        const size_type pos = i - begin();
        const typename std::iterator_traits<FwdIterator>::difference_type n2 = 
            std::distance(s1, s2);
        assert(n2 >= 0);
        using namespace flex_string_details;
        assert(pos <= size());

        const typename std::iterator_traits<FwdIterator>::difference_type maxn2 = 
            capacity() - size();
        if (maxn2 < n2)
        {
            // realloc the string
            static const std::less_equal<const value_type*> le = 
                std::less_equal<const value_type*>();
            assert(!(le(&*begin(), &*s1) && le(&*s1, &*end())));
            reserve(size() + n2);
            i = begin() + pos;
        }
        if (pos + n2 <= size())
        {
            //const iterator oldEnd = end();
            //Storage::append(oldEnd - n2, n2);
            //std::copy(i, oldEnd - n2, i + n2);
            const iterator tailBegin = end() - n2;
            Storage::append(tailBegin, tailBegin + n2);
            //std::copy(i, tailBegin, i + n2);
            std::copy(reverse_iterator(tailBegin), reverse_iterator(i), 
                reverse_iterator(tailBegin + n2));
            std::copy(s1, s2, i);
        }
        else
        {
            FwdIterator t = s1;
            const size_type old_size = size();
            std::advance(t, old_size - pos);
            assert(std::distance(t, s2) >= 0);
            Storage::append(t, s2);
            Storage::append(data() + pos, data() + old_size);
            std::copy(s1, t, i);
        }
    }

    template <class InputIterator>
    void InsertImpl(iterator i1, iterator i2,
        InputIterator b, InputIterator e, std::input_iterator_tag)
    { 
        flex_string temp(begin(), i1);
        for (; b != e; ++b)
        {
            temp.push_back(*b);
        }
        temp.append(i2, end());
        swap(temp);
    }

public:
    template <class ItOrLength, class ItOrChar>
    void insert(iterator p, ItOrLength first_or_n, ItOrChar last_or_c)
    { 
        Selector<std::numeric_limits<ItOrLength>::is_specialized> sel;
        InsertImplDiscr(p, first_or_n, last_or_c, sel);
    }
    
    iterator erase(iterator first, iterator last)
    {
        Invariant<E, T, A, Storage> checker(*this); 
        (void) checker;
		unsigned pos = first - begin();
		unsigned n = last - first;
        Enforce(pos <= size(), static_cast<std::out_of_range*>(0), "");
        Procust<size_type>(n, size() - pos);
        std::copy(begin() + pos + n, end(), begin() + pos);
        resize(size() - n, value_type());
        return begin() + pos;
    }
  
private:
    flex_string& ReplaceImplDiscr(iterator i1, iterator i2, 
        const value_type* s, size_type n, Selector<2>)
    { 
        assert(i1 <= i2);
        assert(begin() <= i1 && i1 <= end());
        assert(begin() <= i2 && i2 <= end());
        return replace(i1, i2, s, s + n); 
    }
    
    flex_string& ReplaceImplDiscr(iterator i1, iterator i2,
        size_type n2, value_type c, Selector<1>)
    { 
        const size_type n1 = i2 - i1;
        if (n1 > n2)
        {
            std::fill(i1, i1 + n2, c);
            erase(i1 + n2, i2);
        }
        else
        {
            std::fill(i1, i2, c);
            insert(i2, n2 - n1, c);
        }
        return *this;
    }    

    template <class InputIterator>
    flex_string& ReplaceImplDiscr(iterator i1, iterator i2,
        InputIterator b, InputIterator e, Selector<0>)
    { 
        ReplaceImpl(i1, i2, b, e, 
            typename std::iterator_traits<InputIterator>::iterator_category());
        return *this;
    }

    template <class FwdIterator>
    void ReplaceImpl(iterator i1, iterator i2,
        FwdIterator s1, FwdIterator s2, std::forward_iterator_tag)
    { 
        Invariant<E, T, A, Storage> checker(*this); 
        (void) checker;
        const typename std::iterator_traits<iterator>::difference_type n1 = 
            i2 - i1;
        assert(n1 >= 0);
        const typename std::iterator_traits<FwdIterator>::difference_type n2 = 
            std::distance(s1, s2);
        assert(n2 >= 0);

        // Handle aliased replace
        static const std::less_equal<const value_type*> le = 
            std::less_equal<const value_type*>();
        const bool aliased = le(&*begin(), &*s1) && le(&*s1, &*end());
        if (aliased /* && capacity() < size() - n1 + n2 */)
        {
            // Aliased replace, copy to new string
            flex_string temp;
            temp.reserve(size() - n1 + n2);
			yasli::append(temp, begin(), i1);
			yasli::append(temp, s1, s1);
			yasli::append(temp, i2, end());
            //temp.append(begin(), i1).append(s1, s2).append(i2, end());
            swap(temp);
            return;
        }

        if (n1 > n2)
        {
            // shrinks
            std::copy(s1, s2, i1);
            erase(i1 + n2, i2);
        }
        else
        {
            // grows
            flex_string_details::copy_n(s1, n1, i1);
            std::advance(s1, n1);
            insert(i2, s1, s2);
        }
    }

    template <class InputIterator>
    void ReplaceImpl(iterator i1, iterator i2,
        InputIterator b, InputIterator e, std::input_iterator_tag)
    {
        flex_string temp(begin(), i1);
        temp.append(b, e).append(i2, end());
        swap(temp);
    }

public:
    template <class T1, class T2>
    flex_string& replace(iterator i1, iterator i2,
        T1 first_or_n_or_s, T2 last_or_c_or_n)
    { 
        const bool 
            num1 = std::numeric_limits<T1>::is_specialized,
            num2 = std::numeric_limits<T2>::is_specialized;
        return ReplaceImplDiscr(i1, i2, first_or_n_or_s, last_or_c_or_n, 
            Selector<num1 ? (num2 ? 1 : -1) : (num2 ? 2 : 0)>()); 
    }

    void swap(flex_string& rhs)
    {
        Storage& srhs = rhs;
        this->Storage::swap(srhs);
    }
    
    // 21.3.6 string operations:
    const value_type* c_str() const
    { return Storage::c_str(); }
    
    const value_type* data() const
    { return Storage::data(); }
    
    allocator_type get_allocator() const
    { return Storage::get_allocator(); }
    
};

template<class E, class T, class A, class S>
typename A::size_type find (const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, 
							typename flex_string<E, T, A, S>::size_type pos, typename flex_string<E, T, A, S>::size_type n) 
{
    if (n + pos > s1.size())
		return flex_string<E, T, A, S>::npos;
    for (; pos + n <= s1.size(); ++pos)
    {
		if (flex_string<E, T, A, S>::traits_type::compare(s1.data() + pos, s, n) == 0)
        {
            return pos;
        }
    }
	return flex_string<E, T, A, S>::npos;
}

template<class E, class T, class A, class S>
typename A::size_type find(const flex_string<E, T, A, S>& s1, const flex_string<E, T, A, S>& str, typename A::size_type pos = 0) 
{ return find(s1, str.data(), pos, length(str)); }

template<class E, class T, class A, class S>
typename A::size_type find(const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, typename A::size_type pos = 0)
{ return find(s1, s, pos, flex_string<E, T, A, S>::traits_type::length(s)); }

template<class E, class T, class A, class S>
typename A::size_type find(const flex_string<E, T, A, S>& s1, typename flex_string<E, T, A, S>::value_type c, typename flex_string<E, T, A, S>::size_type pos = 0)
{ return find(s1, &c, pos, 1); }

template<class E, class T, class A, class S>
typename A::size_type rfind(const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, typename flex_string<E, T, A, S>::size_type pos, typename flex_string<E, T, A, S>::size_type n) 
{
	if (n > length(s1)) return flex_string<E, T, A, S>::npos;
    pos = Min(pos, length(s1) - n);
    if (n == 0) return pos;

	typename flex_string<E, T, A, S>::const_iterator i(s1.begin() + pos);
    for (; ; --i)
    {
        if (flex_string<E, T, A, S>::traits_type::eq(*i, *s) 
            && flex_string<E, T, A, S>::traits_type::compare(&*i, s, n) == 0)
        {
            return i - s1.begin();
        }
        if (i == s1.begin()) break;
    }
    return flex_string<E, T, A, S>::npos;
}

template<class E, class T, class A, class S>
typename A::size_type rfind(const flex_string<E, T, A, S>& s1, const flex_string<E, T, A, S>& str, typename A::size_type pos = npos) 
{ return rfind(s1, str.data(), pos, length(str)); }

template<class E, class T, class A, class S>
typename A::size_type rfind(const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, typename flex_string<E, T, A, S>::size_type pos = flex_string<E, T, A, S>::npos) 
{ return rfind(s1, s, pos, flex_string<E, T, A, S>::traits_type::length(s)); }

template<class E, class T, class A, class S>
typename A::size_type rfind(const flex_string<E, T, A, S>& s1, typename flex_string<E, T, A, S>::value_type c, typename A::size_type pos = flex_string<E, T, A, S>::npos) 
{ return rfind(s1, &c, pos, 1); }

template<class E, class T, class A, class S>
typename A::size_type find_first_of(const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, 
    typename A::size_type pos, typename A::size_type n) 
{
    if (pos > length(s1) || n == 0) return flex_string<E, T, A, S>::npos;
    typename flex_string<E, T, A, S>::const_iterator i(s1.begin() + pos),
        finish(s1.end());
    for (; i != finish; ++i)
    {
        if (flex_string<E, T, A, S>::traits_type::find(s, n, *i) != 0)
        {
            return i - s1.begin();
        }
    }
    return flex_string<E, T, A, S>::npos;
}

template<class E, class T, class A, class S>
typename A::size_type find_first_of(const flex_string<E, T, A, S>& s1, const flex_string<E, T, A, S>& str, typename A::size_type pos = 0) 
{ return find_first_of(s1, str.data(), pos, length(str)); }
    
template<class E, class T, class A, class S>
typename A::size_type find_first_of(const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, typename A::size_type pos = 0) 
{ return find_first_of(s1, s, pos, flex_string<E, T, A, S>::traits_type::length(s)); }

template<class E, class T, class A, class S>
typename A::size_type find_first_of(const flex_string<E, T, A, S>& s1, typename flex_string<E, T, A, S>::value_type c, typename A::size_type pos = 0) 
{ return find_first_of(s1, &c, pos, 1); }

template<class E, class T, class A, class S>
typename A::size_type find_last_of (const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, typename A::size_type pos, 
    typename A::size_type n) 
{
    if (!empty(s1) && n > 0)
    {
        pos = Min(pos, length(s1) - 1);
        typename flex_string<E, T, A, S>::const_iterator i(s1.begin() + pos);
        for (;; --i)
        {
            if (flex_string<E, T, A, S>::traits_type::find(s, n, *i) != 0)
            {
                return i - s1.begin();
            }
            if (i == s1.begin()) break;
        }
    }
    return flex_string<E, T, A, S>::npos;
}

template<class E, class T, class A, class S>
typename A::size_type find_last_of (const flex_string<E, T, A, S>& s1, const flex_string<E, T, A, S>& str,
    typename A::size_type pos = flex_string<E, T, A, S>::npos) 
{ return find_last_of(s1, str.data(), pos, length(str)); }

template<class E, class T, class A, class S>
typename A::size_type find_last_of (const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, 
    typename A::size_type pos = flex_string<E, T, A, S>::npos) 
{ return find_last_of(s1, s, pos, flex_string<E, T, A, S>::traits_type::length(s)); }

template<class E, class T, class A, class S>
typename A::size_type find_last_of (const flex_string<E, T, A, S>& s1, typename flex_string<E, T, A, S>::value_type c, typename A::size_type pos = flex_string<E, T, A, S>::npos) 
{ return find_last_of(s1, &c, pos, 1); }

template<class E, class T, class A, class S>
typename A::size_type find_first_not_of(const flex_string<E, T, A, S>& s1, const flex_string<E, T, A, S>& str,
    typename A::size_type pos = 0) 
{ return find_first_not_of(s1, str.data(), pos, str.size()); }

template<class E, class T, class A, class S>
typename A::size_type find_first_not_of(const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, typename A::size_type pos,
    typename A::size_type n) 
{
    if (pos < length(s1))
    {
        typename flex_string<E, T, A, S>::const_iterator 
            i(s1.begin() + pos),
            finish(s1.end());
        for (; i != finish; ++i)
        {
            if (flex_string<E, T, A, S>::traits_type::find(s, n, *i) == 0)
            {
                return i - s1.begin();
            }
        }
    }
    return flex_string<E, T, A, S>::npos;
}

template<class E, class T, class A, class S>
typename A::size_type find_first_not_of(const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, 
    typename A::size_type pos = 0) 
{ return find_first_not_of(s1, s, pos, flex_string<E, T, A, S>::traits_type::length(s)); }
    
template<class E, class T, class A, class S>
typename A::size_type find_first_not_of(const flex_string<E, T, A, S>& s1, typename flex_string<E, T, A, S>::value_type c, typename A::size_type pos = 0) 
{ return find_first_not_of(s1, &c, pos, 1); }

template<class E, class T, class A, class S>
typename A::size_type find_last_not_of(const flex_string<E, T, A, S>& s1, const flex_string<E, T, A, S>& str,
    typename A::size_type pos = flex_string<E, T, A, S>::npos) 
{ return find_last_not_of(s1, str.data(), pos, length(str)); }

template<class E, class T, class A, class S>
typename A::size_type find_last_not_of(const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, typename A::size_type pos,
    typename A::size_type n) 
{
    if (!empty(s1))
    {
        pos = Min(pos, s1.size() - 1);
        typename flex_string<E, T, A, S>::const_iterator i(s1.begin() + pos);
        for (;; --i)
        {
            if (flex_string<E, T, A, S>::traits_type::find(s, n, *i) == 0)
            {
                return i - s1.begin();
            }
            if (i == s1.begin()) break;
        }
    }
    return flex_string<E, T, A, S>::npos;
}

template<class E, class T, class A, class S>
typename A::size_type find_last_not_of(const flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s, 
    typename A::size_type pos = flex_string<E, T, A, S>::npos) 
{ return find_last_not_of(s1, s, pos, flex_string<E, T, A, S>::traits_type::length(s)); }

template<class E, class T, class A, class S>
typename A::size_type find_last_not_of (const flex_string<E, T, A, S>& s1, typename flex_string<E, T, A, S>::value_type c, typename A::size_type pos = flex_string<E, T, A, S>::npos) 
{ return find_last_not_of(s1, &c, pos, 1); }

// Replaces at most n1 chars of *this, starting with pos, with n2 occurences of c
// consolidated with
// Replaces at most n1 chars of *this, starting with pos,
// with at most n2 chars of str.
// str must have at least n2 chars.
template<class E, class T, class A, class S, class StrOrLength, class NumOrChar>
flex_string<E, T, A, S>& replace(flex_string<E, T, A, S>& s1, typename A::size_type pos, typename A::size_type n1, 
					 StrOrLength s_or_n2, NumOrChar n_or_c)
{
	Invariant<E,T,A,S> checker(s1); 
	(void) checker;
	Enforce(pos <= s1.size(), static_cast<std::out_of_range*>(0), "");
	Procust(n1, length(s1) - pos);
	const typename flex_string<E, T, A, S>::iterator b = s1.begin() + pos;
	return s1.replace(b, b + n1, s_or_n2, n_or_c);
}
   // Replaces at most n1 chars of *this, starting with pos1 with the content of str
template<class E, class T, class A, class S>
flex_string<E, T, A, S>& replace(flex_string<E, T, A, S>& s1, typename A::size_type pos1, typename A::size_type n1, const flex_string<E, T, A, S>& str)
{ return replace(s1, pos1, n1, str.data(), str.size()); }

// Replaces at most n1 chars of *this, starting with pos1,
// with at most n2 chars of str starting with pos2
template<class E, class T, class A, class S>
flex_string<E, T, A, S>& replace(flex_string<E, T, A, S>& s1, typename A::size_type pos1, typename A::size_type n1, const flex_string<E, T, A, S>& str,
								 typename A::size_type pos2, typename A::size_type n2)
{
    Enforce(pos2 <= str.size(), static_cast<std::out_of_range*>(0), "");
    return replace(s1, pos1, n1, str.data() + pos2, 
        Min(n2, str.size() - pos2));
}
 // Replaces at most n1 chars of *this, starting with pos, with chars from s
template<class E, class T, class A, class S>
flex_string<E, T, A, S>& replace(flex_string<E, T, A, S>& s1, typename flex_string<E, T, A, S>::size_type pos, typename flex_string<E, T, A, S>::size_type n1, const typename flex_string<E, T, A, S>::value_type* s)
{ return replace(s1, pos, n1, s, flex_string<E, T, A, S>::traits_type::length(s)); }


template<class E, class T, class A, class S>
flex_string<E, T, A, S>& replace(flex_string<E, T, A, S> &s1, typename flex_string<E, T, A, S>::iterator i1, 
								 typename flex_string<E, T, A, S>::iterator i2, const flex_string<E, T, A, S>& str)
{ return s1.replace(i1, i2, str.data(), str.size()); }

template<class E, class T, class A, class S>
flex_string<E, T, A, S>& replace(flex_string<E, T, A, S> &s1, typename flex_string<E, T, A, S>::iterator i1, typename flex_string<E, T, A, S>::iterator i2, const typename flex_string<E, T, A, S>::value_type* s)
{ return s1.replace(i1, i2, s, flex_string<E, T, A, S>::traits_type::length(s)); }

// Code from Jean-Francois Bastien (03/26/2007)
template<class E, class T, class A, class S>
int compare(const flex_string<E, T, A, S>& str, const typename flex_string<E, T, A, S>::value_type* s) 
{
	// Could forward to compare(0, size(), s, traits_type::length(s))
	// but that does two extra checks
	const typename flex_string<E, T, A, S>::size_type n1(str.size()), n2(flex_string<E, T, A, S>::traits_type::length(s));
	const int r = flex_string<E, T, A, S>::traits_type::compare(str.data(), s, Min(n1, n2));
	return r != 0 ? r : n1 > n2 ? 1 : n1 < n2 ? -1 : 0;
}   

template<class E, class T, class A, class S>
int compare(const flex_string<E, T, A, S>& str, typename A::size_type pos1, typename A::size_type n1,
			const typename flex_string<E, T, A, S>::value_type* s, typename A::size_type n2) 
{
    Enforce(pos1 <= str.size(), static_cast<std::out_of_range*>(0), "");
	Procust(n1, str.size() - pos1);
	// The line below fixed by Jean-Francois Bastien, 04-23-2007. Thanks!
	const int r = flex_string<E, T, A, S>::traits_type::compare(pos1 + str.data(), s, Min(n1, n2));
	return r != 0 ? r : n1 > n2 ? 1 : n1 < n2 ? -1 : 0;
}

template<class E, class T, class A, class S>
int compare(const flex_string<E, T, A, S>& lhs, typename A::size_type pos1, typename A::size_type n1,
    const flex_string<E, T, A, S>& rhs) 
{ return compare(lhs, pos1, n1, rhs.data(), rhs.size()); }

template<class E, class T, class A, class S>
int compare(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>& rhs) 
{ 
    // FIX due to Goncalo N M de Carvalho July 18, 2005
    return compare(lhs, 0, lhs.size(), rhs);
}

// FIX to compare: added the TC 
// (http://www.comeaucomputing.com/iso/lwg-defects.html number 5)
// Thanks to Caleb Epstein for the fix
template<class E, class T, class A, class S>
int compare(const flex_string<E, T, A, S>& str, typename A::size_type pos1, typename A::size_type n1,
			const typename flex_string<E, T, A, S>::value_type* s)
{
    return compare(str, pos1, n1, s, traits_type::length(s));
}


template<class E, class T, class A, class S>
int compare(const flex_string<E, T, A, S>& lhs, typename A::size_type pos1, typename A::size_type n1,
    const flex_string<E, T, A, S>& rhs,
    typename A::size_type pos2, typename A::size_type n2)
{
    Enforce(pos2 <= rhs.size(), static_cast<std::out_of_range*>(0), "");
    return compare(lhs, pos1, n1, rhs.data() + pos2, Min(n2, rhs.size() - pos2));
}

template<class E, class T, class A, class S>
flex_string<E, T, A, S> substr(const flex_string<E, T, A, S>& s1, typename A::size_type pos = 0, typename A::size_type n = flex_string<E, T, A, S>::npos) 
{
    Enforce(pos <= s1.size(), static_cast<std::out_of_range*>(0), "");
	return flex_string<E, T, A, S>(s1.data() + pos, Min< A::size_type>(n, s1.size() - pos));
}

template<class E, class T, class A, class S>
typename flex_string<E, T, A, S>::size_type copy(const flex_string<E, T, A, S>& s1, typename flex_string<E, T, A, S>::value_type* s, typename flex_string<E, T, A, S>::size_type n, typename flex_string<E, T, A, S>::size_type pos = 0) 
{
    Enforce(pos <= s1.size(), static_cast<std::out_of_range*>(0), "");
    Procust(n, s1.size() - pos);
    
    flex_string_details::pod_copy(
        s1.data() + pos,
        s1.data() + pos + n,
        s);
    return n;
}

template<class E, class T, class A, class S>
flex_string<E, T, A, S>&
erase (
	flex_string<E, T, A, S>& s,
	typename A::size_type pos = 0,
	typename A::size_type n = flex_string<E, T, A, S>::npos
	   )
{
	if (pos > s.size())
	{
		throw std::out_of_range("pos out_of_range!");
	}
	typename flex_string<E, T, A, S>::iterator 
		first = s.begin() + pos,
		last =  n == flex_string<E, T, A, S>::npos? s.end():first + min(n, s.size()-pos);
	if (first != last)
	{
		s.erase(first, last);
	}
	return s;
}

template <typename E, class T, class A, class S>
typename flex_string<E, T, A, S>::iterator erase(flex_string<E, T, A, S>&s1, typename flex_string<E, T, A, S>::iterator pos)
{
	return s1.erase(pos, pos+1);
}

template <typename E, class T, class A, class S>
bool Sane(const flex_string<E, T, A, S>& s1)
{
	return 
		s1.begin() <= s1.end() &&
		empty(s1) == (s1.size() == 0) &&
		empty(s1) == (s1.begin() == s1.end()) &&
		s1.size() <= s1.max_size() &&
		s1.capacity() <= s1.max_size() &&
		s1.size() <= s1.capacity();
}

template <typename E, class T, class A, class S>
struct Invariant
{
#ifndef NDEBUG
    Invariant(const flex_string<E, T, A, S>& s) : s_(s)
    {
        assert(Sane(s_));
    }
    ~Invariant()
    {
        assert(Sane(s_));
    }
private:
    const flex_string<E, T, A, S>& s_;
#else
    Invariant(const flex_string<E, T, A, S>&) {} 
#endif
	Invariant& operator=(const Invariant&);
};
    
 // 21.3.5 modifiers:
template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& append(flex_string<E, T, A, S>& s1, const flex_string<E, T, A, S>& str)
{ return s1.append(str.data(), length(str)); }
    
template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& append(flex_string<E, T, A, S>& s1, const flex_string<E, T, A, S>& str, const typename A::size_type pos, typename A::size_type n)
{ 
	const A::size_type sz = str.size();
    Enforce(pos <= sz, static_cast<std::out_of_range*>(0), "");
    Procust(n, sz - pos);
    return s1.append( str.data() + pos, n); 
}
    
template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& append(flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s)
{ return s1.append(s, flex_string<E, T, A, S>::traits_type::length(s)); }

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& append(flex_string<E, T, A, S>& s1, typename flex_string<E, T, A, S>::size_type n, typename flex_string<E, T, A, S>::value_type c)
{ 
    s1.resize(s1.size() + n, c);
    return s1;
}

template <typename E, class T, class A, class S, class InputIterator>
flex_string<E, T, A, S>& append(flex_string<E, T, A, S>& s1, InputIterator first, InputIterator last)
{
    s1.insert(s1.end(), first, last);
    return s1;
}
    
template<class T>
typename T::size_type length(const T& t) 
{ return t.size(); }

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& insert(flex_string<E, T, A, S>& s1, typename A::size_type pos, const typename flex_string<E, T, A, S>::value_type* s, typename A::size_type n)
{ 
    Enforce(pos <= length(s1), static_cast<std::out_of_range*>(0), "");
    s1.insert(s1.begin() + pos, s, s + n); 
    return s1;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& insert(flex_string<E, T, A, S>& s, typename A::size_type pos1, const flex_string<E, T, A, S>& str)//0
{ return insert(s, pos1, str.data(), str.size()); }

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& insert(flex_string<E, T, A, S>& s, typename A::size_type pos1, const flex_string<E, T, A, S>& str,
    typename A::size_type pos2, typename A::size_type n)
{ 
    Enforce(pos2 <= length(str), static_cast<std::out_of_range*>(0), "");
    Procust(n, length(str) - pos2);
    return insert(s, pos1, str.data() + pos2, n); 
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& insert(flex_string<E, T, A, S>& s1, typename A::size_type pos, const typename flex_string<E, T, A, S>::value_type* s)
{ return insert(s1, pos, s, flex_string<E, T, A, S>::traits_type::length(s)); }

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& insert(flex_string<E, T, A, S>& s1, typename A::size_type pos, typename A::size_type n, typename flex_string<E, T, A, S>::value_type c)
{
    Enforce(pos <= length(s1), static_cast<std::out_of_range*>(0), "");
    s1.insert(s1.begin() + pos, n, c);
    return s1;
}

template <typename E, class T, class A, class S>
typename flex_string<E, T, A, S>::iterator
insert(flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::iterator p, const typename flex_string<E, T, A, S>::value_type c) 
{
    const typename A::size_type pos = p - s1.begin();
    s1.insert(p, 1, c);
    return s1.begin() + pos;
}

template <typename E, class T, class A, class S>
void push_back(flex_string<E, T, A, S>& s, E c)
{
	insert(s, s.end(), c);
}

template <typename E, class T, class A, class S>
void resize(flex_string<E, T, A, S>& s, typename flex_string<E, T, A, S>::size_type n, typename flex_string<E, T, A, S>::value_type c)
{
	s.resize(n,c);
}

template <typename E, class T, class A, class S>
void resize(flex_string<E, T, A, S>& s, typename A::size_type n)
{
	resize(s,n,E());
}

template <typename T>
bool empty( const T& t)
{
	return t.begin() == t.end();
}

template <typename E, class T, class A, class S>
void clear(flex_string<E, T, A, S>&s) 
{
	s.erase(s.begin(), s.end());
}

template <typename E, class T, class A, class S>
typename flex_string<E, T, A, S>::const_reference at(const flex_string<E, T, A, S>&s, typename flex_string<E, T, A, S>::size_type n ) 
{
	Enforce(n <= s.size(), static_cast<std::out_of_range*>(0), "");
    return s[n];
}

template <typename E, class T, class A, class S>
typename flex_string<E, T, A, S>::reference at(flex_string<E, T, A, S>&s, typename flex_string<E, T, A, S>::size_type n ) 
{
	Enforce(n <= s.size(), static_cast<std::out_of_range*>(0), "");
    return s[n];
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& operator += (flex_string<E, T, A, S>& s, const flex_string<E, T, A, S>& str)
{
	return append(s,str);
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& operator += (flex_string<E, T, A, S>& s, const E* str)
{
	return append(s,str);
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& operator += (flex_string<E, T, A, S>& s, const E c)
{
	push_back(s,c);
	return s;
}

// non-member functions
template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{
    flex_string<E, T, A, S> result;
    result.reserve(lhs.size() + rhs.size());
	append(result, lhs);
	append(result, rhs);
    return result;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{
    flex_string<E, T, A, S> result;
    const typename flex_string<E, T, A, S>::size_type len = 
        flex_string<E, T, A, S>::traits_type::length(lhs);
    result.reserve(len + rhs.size());
	result.append(lhs, len);
	append(result, rhs);
    return result;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(
    typename flex_string<E, T, A, S>::value_type lhs, 
    const flex_string<E, T, A, S>& rhs)
{
    flex_string<E, T, A, S> result;
    result.reserve(1 + rhs.size());
    push_back(result, lhs);
	append(result, rhs);
    return result;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{
    typedef typename flex_string<E, T, A, S>::size_type size_type;
    typedef typename flex_string<E, T, A, S>::traits_type traits_type;

    flex_string<E, T, A, S> result;
	const typename A::size_type len = traits_type::length(rhs);
    result.reserve(lhs.size() + len);
	append(result, lhs);
	result.append(rhs, len);
    return result;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(const flex_string<E, T, A, S>& lhs, 
    typename flex_string<E, T, A, S>::value_type rhs)
{
    flex_string<E, T, A, S> result;
    result.reserve(lhs.size() + 1);
    append(result, lhs);
    push_back(result, rhs);
    return result;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& assign(flex_string<E, T, A, S>& s1, const  typename flex_string<E, T, A, S>::value_type* s, const typename flex_string<E, T, A, S>::size_type n)
{
    Invariant<E, T, A, S> checker(s1); 
    (void) checker; 
    if (s1.size() >= n)
    {
        std::copy(s, s + n, s1.begin());
        resize(s1, n);
    }
    else
    {
        const typename flex_string<E, T, A, S>::value_type *const s2 = s + s1.size();
        std::copy(s, s2, s1.begin());
        s1.append(s2, n - s1.size());
    }
    return s1;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& assign(flex_string<E, T, A, S>& s1, const flex_string<E, T, A, S>& str)
{ 
    if (&str == &s1) return s1;
    return assign(s1, str.data(), str.size());
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& assign(flex_string<E, T, A, S>& s1, const flex_string<E, T, A, S>& str, const typename flex_string<E, T, A, S>::size_type pos,
								typename flex_string<E, T, A, S>::size_type n)
{ 
    const typename flex_string<E, T, A, S>::size_type  sz = str.size();
    Enforce(pos <= sz, static_cast<std::out_of_range*>(0), "");
    Procust(n, sz - pos);
    return assign(s1, str.data() + pos, n);
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S>& assign(flex_string<E, T, A, S>& s1, const typename flex_string<E, T, A, S>::value_type* s)
{ return assign(s1, s, flex_string<E, T, A, S>::traits_type::length(s)); }
    
template <typename E, class T, class A, class S, class ItOrLength, class ItOrChar>
flex_string<E, T, A, S>& assign(flex_string<E, T, A, S>& s1, ItOrLength first_or_n, ItOrChar last_or_c)
{ return s1.replace(s1.begin(), s1.end(), first_or_n, last_or_c); }
    
template <typename E, class T, class A, class S>
bool operator==(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return compare(lhs, rhs) == 0; }

template <typename E, class T, class A, class S>
bool operator==(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return rhs == lhs; }

template <typename E, class T, class A, class S>
bool operator==(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return compare(lhs,rhs) == 0; }

template <typename E, class T, class A, class S>
bool operator!=(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(lhs == rhs); }

template <typename E, class T, class A, class S>
bool operator!=(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(lhs == rhs); }

template <typename E, class T, class A, class S>
bool operator!=(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return !(lhs == rhs); }

template <typename E, class T, class A, class S>
bool operator<(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return compare(lhs,rhs) < 0; }

template <typename E, class T, class A, class S>
bool operator<(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return compare(lhs,rhs) < 0; }

template <typename E, class T, class A, class S>
bool operator<(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return compare(rhs,lhs) > 0; }

template <typename E, class T, class A, class S>
bool operator>(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return rhs < lhs; }

template <typename E, class T, class A, class S>
bool operator>(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return rhs < lhs; }

template <typename E, class T, class A, class S>
bool operator>(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return rhs < lhs; }

template <typename E, class T, class A, class S>
bool operator<=(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(rhs < lhs); }

template <typename E, class T, class A, class S>
bool operator<=(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return !(rhs < lhs); }

template <typename E, class T, class A, class S>
bool operator<=(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(rhs < lhs); }

template <typename E, class T, class A, class S>
bool operator>=(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(lhs < rhs); }

template <typename E, class T, class A, class S>
bool operator>=(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return !(lhs < rhs); }

template <typename E, class T, class A, class S>
bool operator>=(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(lhs < rhs); }

// subclause 21.3.7.8:
//void swap(flex_string<E, T, A, S>& lhs, flex_string<E, T, A, S>& rhs);    // to do
template <typename E, class T, class A, class S>
void swap(flex_string<E, T, A, S>& lhs, flex_string<E, T, A, S>& rhs) 
{
	lhs.swap(rhs);
}
//void* checkedRealloc(void* ptr, size_t size) {
//	void* p = realloc(ptr, size);
//	if (!p) throw std::bad_alloc();
//	return p;
//}

template <typename E, class T, class A, class S>
std::basic_istream<typename flex_string<E, T, A, S>::value_type, 
    typename flex_string<E, T, A, S>::traits_type>&
operator>>(
    std::basic_istream<typename flex_string<E, T, A, S>::value_type, 
	typename flex_string<E, T, A, S>::traits_type>& is,
    flex_string<E, T, A, S>& str)
{	// extract a string
	typedef std::ctype<E> Ctype;
	typedef std::basic_istream<E, T> Myis;
	typedef flex_string<E, T, A, S> Mystr;
	typedef typename Mystr::size_type Mysizt;

	std::ios_base::iostate State = std::ios_base::goodbit;
	bool Changed = false;
	const typename Myis::sentry Ok(is);

	if (Ok)
	{	// state okay, extract characters
		const Ctype& Ctype_fac = std::use_facet <Ctype>(is.getloc());
		//str.erase();
		clear(str);

		try {
			Mysizt Size = 0 < is.width()
			&& (Mysizt)is.width() < str.max_size()
			? (Mysizt)is.width() : str.max_size();
		typename T::int_type Meta = is.rdbuf()->sgetc();

		for (; 0 < Size; --Size, Meta = is.rdbuf()->snextc())
			if(T::eq_int_type(T::eof(), Meta))
			{	// end of file, quit
				State |= std::ios_base::eofbit;
				break;
			}
			else if (Ctype_fac.is(Ctype::space,
				T::to_char_type(Meta)))
				break;	// whitespace, quit
			else
			{	// add character to string
				append(str, 1, T::to_char_type(Meta));
				Changed = true;
			}
		} catch (...) {
			is.setstate(std::ios_base::badbit, true);
		}
	}

	is.width(0);
	if (!Changed)
		State |= std::ios_base::failbit;
	is.setstate(State);
	return (is);
}
//
//template<class E,
//	class T,
//	class A, class S> 
//std::basic_ostream<typename flex_string<E, T, A, S>::value_type,
//    typename flex_string<E, T, A, S>::traits_type>&
//operator<<(
//    std::basic_ostream<typename flex_string<E, T, A, S>::value_type, 
//		typename flex_string<E, T, A, S>::traits_type>& os,
//    const flex_string<E, T, A, S>& str)
//{	// insert a string
//	typedef std::basic_ostream<E, T> Myos;
//	typedef flex_string<E, T, A, S> Mystr;
//	typedef typename Mystr::size_type Mysizt;
//
//	ios_base::iostate State = ios_base::goodbit;
//	Mysizt Size = str.size();
//	Mysizt Pad = os.width() <= 0 || (Mysizt)os.width() <= Size
//			? 0 : (Mysizt)os.width() - Size;
//	const typename Myos::sentry Ok1(os);
//
//	if (!Ok1)
//		State |= ios_base::badbit;
//	else
//		{	// state okay, insert characters
//		try {
//		if ((os.flags() & ios_base::adjustfield) != ios_base::left)
//			for (; 0 < Pad; --Pad)	// pad on left
//				if (T::eq_int_type(T::eof(),
//					os.rdbuf()->sputc(os.fill())))
//					{	// insertion failed, quit
//					State |= ios_base::badbit;
//					break;
//					}
//
//		if (State == ios_base::goodbit)
//			for (Mysizt Count = 0; Count < Size; ++Count)
//				if (T::eq_int_type(T::eof(),
//					os.rdbuf()->sputc(str[Count])))
//					{	// insertion failed, quit
//					State |= ios_base::badbit;
//					break;
//					}
//
//		if (State == ios_base::goodbit)
//			for (; 0 < Pad; --Pad)	// pad on right
//				if (T::eq_int_type(T::eof(),
//					os.rdbuf()->sputc(os.fill())))
//					{	// insertion failed, quit
//					State |= ios_base::badbit;
//					break;
//					}
//		os.width(0);
//		} catch (...) {
//			os.setstate(ios_base::badbit, true);
//		}
//		}
//
//	os.setstate(State);
//	return (os);
//	}
//
template <typename E, class T, class A, class S>
std::basic_istream<typename flex_string<E, T, A, S>::value_type,
    typename flex_string<E, T, A, S>::traits_type>&
getline(
    std::basic_istream<typename flex_string<E, T, A, S>::value_type, 
        typename flex_string<E, T, A, S>::traits_type>& is,
    flex_string<E, T, A, S>& str,
    typename flex_string<E, T, A, S>::value_type delim)

{	// get characters into string, discard delimiter
	typedef std::basic_istream<E, T> Myis;
	std::ios_base::iostate State = std::ios_base::goodbit;
	bool Changed = false;
	const typename Myis::sentry Ok(is, true);

	if (Ok)
		{	// state okay, extract characters
		try {
		clear(str);
		const typename T::int_type Metadelim =
			T::to_int_type(delim);
		typename T::int_type Meta = is.rdbuf()->sgetc();

		for (; ; Meta = is.rdbuf()->snextc())
			if (T::eq_int_type(T::eof(), Meta))
				{	// end of file, quit
				State |= std::ios_base::eofbit;
				break;
				}
			else if (T::eq_int_type(Meta, Metadelim))
				{	// got a delimiter, discard it and quit
				Changed = true;
				is.rdbuf()->sbumpc();
				break;
				}
			else if (str.max_size() <= str.size())
				{	// string too large, quit
				State |= std::ios_base::failbit;
				break;
				}
			else
				{	// got a character, add it to string
				str += T::to_char_type(Meta);
				Changed = true;
				}
		} catch (...) {
			is.setstate(std::ios_base::badbit, true);
		}
	}

	if (!Changed)
		State |= std::ios_base::failbit;
	is.setstate(State);
	return (is);
}

template <typename E, class T, class A, class S>
std::basic_ostream<typename flex_string<E, T, A, S>::value_type,
                   typename flex_string<E, T, A, S>::traits_type>&
operator<<(
  std::basic_ostream<typename flex_string<E, T, A, S>::value_type,
  typename flex_string<E, T, A, S>::traits_type>& os,
    const flex_string<E, T, A, S>& str) {
  os.write(str.data(), str.size());
  return os;
}
template <typename E, class T, class A, class S>
std::basic_istream<typename flex_string<E, T, A, S>::value_type, 
    typename flex_string<E, T, A, S>::traits_type>&
getline(
    std::basic_istream<typename flex_string<E, T, A, S>::value_type, 
        typename flex_string<E, T, A, S>::traits_type>& is,
    flex_string<E, T, A, S>& str)
{
	return getline(is, str, is.widen('\n'));
}

template <typename E1, class T, class A, class S>
const typename flex_string<E1, T, A, S>::size_type
flex_string<E1, T, A, S>::npos = static_cast<typename flex_string<E1, T, A, S>::size_type>(-1);
	}
#endif // FLEX_STRING_SHELL_INC_
