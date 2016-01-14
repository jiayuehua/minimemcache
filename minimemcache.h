#ifndef MINIMEMCACHE_H
#define MINIMEMCACHE_H
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <boost/functional/hash.hpp>

#include <iostream>
namespace minimemcache
{
template <class Key , class T, int size>
class Cache;
template<class Key, class T, int size  >
std::ostream& operator<<(std::ostream&os, const Cache<Key, T, size>&cache);

namespace intrusive = boost::intrusive;
template <class Key, class V>
class ID_Attr;
template <class Key, class V>
std::ostream& operator<<(std::ostream&os, const ID_Attr<Key, V> &b);
template <class Key, class V>
class ID_Attr
{
  friend std::ostream& operator<< <>(std::ostream&os, const ID_Attr<Key, V> &b);
  std::pair<Key, V> id_name_;

public:
  explicit ID_Attr(const Key& data, const V& s = V()): id_name_(data, s)
  {
  }
  explicit ID_Attr(const std::pair<Key, V> &p): id_name_(p)
  {
  }
  const V & get() const  { return id_name_.second;    }
  V & get()  { return id_name_.second;    }
  void set(const V &v) {id_name_.second = v;    }

  //This class can be inserted in an intrusive list
  intrusive::list_member_hook<>   list_hook_;

  //This class can be inserted in an intrusive unordered_set
  intrusive::unordered_set_member_hook<>   unordered_set_hook_;

  //Comparison operators
  friend bool operator==(const ID_Attr<Key, V> &a, const ID_Attr<Key, V> &b)
  {  return a.id_name_.first == a.id_name_.first; }


  friend bool operator!=(const ID_Attr<Key, V> &a, const ID_Attr<Key, V> &b)
  {  return a.id_name_.first != b.id_name_.first; }

  //The hash function
  friend std::size_t hash_value(const ID_Attr<Key, V> &i)
  {  return boost::hash<Key>()(i.id_name_.first);  }
};

template <class Key, class V>
std::ostream& operator<< (std::ostream&os, const ID_Attr<Key, V> &b)
{
  os << b.id_name_.first << ", " << b.id_name_.second << "\n";
  return os;
}

template <class T>
struct delete_disposer {
  void operator()(T *delete_this)
  {  delete delete_this;  }
};

template<class Key, class T , int size = 901>
class Cache
{
  //friend std::ostream& operator<<(std::ostream&os, const Cache<ID, Attribute, size>&cache);
public:
  /*void print(std::ostream & os)const
  {
    typedef ID_Attr<ID, Attribute> ID_Attribute;
    typename list_t::const_iterator it = list_.begin();
    typename list_t::const_iterator ite = list_.end();

    for (; it != ite; ++it) {
      std::cout << *it;
    }
  }*/
  typedef Key key_type;
  typedef T mapped_type;
  typedef std::pair<Key, T> value_type;

private:
  friend std::ostream& operator<< <>(std::ostream&os, const Cache<Key, T, size>&cache);
  //Definition of the intrusive list that will hold ID_Attribute
  typedef ID_Attr<Key, T> ID_Attribute;
  typedef intrusive::member_hook<ID_Attribute, intrusive::list_member_hook<>
  , &ID_Attribute::list_hook_> MemberListOption;
  //Definition of the intrusive unordered_set that will hold ID_Attribute
  typedef intrusive::member_hook
  < ID_Attribute, intrusive::unordered_set_member_hook<>
  , &ID_Attribute::unordered_set_hook_> MemberUsetOption;
  typedef intrusive::unordered_set
  < ID_Attribute, MemberUsetOption> unordered_set_t;
  typedef intrusive::list<ID_Attribute, MemberListOption> list_t;
public:

  Cache(): unordered_set_(typename unordered_set_t::bucket_traits(buckets_, size)) {}
  std::pair<bool, T> get(int id)
  {
    typename unordered_set_t::iterator unordered_set_it = unordered_set_.find(ID_Attribute(id));

    if (unordered_set_it == unordered_set_.end()) {
      return std::make_pair(false, T());
    } else {
      typename list_t::iterator l_it = list_.iterator_to(*unordered_set_it);
      list_.erase(l_it);
      list_.push_back(*unordered_set_it);
      return std::make_pair(true, unordered_set_it -> get());
    }
  }

  T& operator [] (const Key& id)
  {

    typename unordered_set_t::iterator unordered_set_it = unordered_set_.find(ID_Attribute(id));

    if (unordered_set_it != unordered_set_.end()) {
      list_.erase(list_.iterator_to(*unordered_set_it));
      list_.push_back(*unordered_set_it);
      return unordered_set_it-> get();
    }else{
      ID_Attribute * pnode = new ID_Attribute(id);
      list_.push_back(*pnode);
      unordered_set_.insert(*pnode);
      return pnode->get();
    }
  }
  void insert(const Key& id, const T& s)
  {
    typename unordered_set_t::iterator unordered_set_it = unordered_set_.find(ID_Attribute(id));

    if (unordered_set_it != unordered_set_.end()) {
      unordered_set_it->set(s);
      list_.erase(list_.iterator_to(*unordered_set_it));
      list_.push_back(*unordered_set_it);
    }else{
      ID_Attribute * pnode = new ID_Attribute(id, s);
      list_.push_back(*pnode);
      unordered_set_.insert(*pnode);
    }
  }

  bool erase(const Key& id)
  {
    if (list_.empty()) {
      return false;
    }

    typename unordered_set_t::iterator unordered_set_it = unordered_set_.find(ID_Attribute(id));

    if (unordered_set_it != unordered_set_.end()) {
      unordered_set_.erase(*unordered_set_it);
      list_.erase_and_dispose(list_.iterator_to(*unordered_set_it), delete_disposer<ID_Attribute>());
      return true;
    } else {
      return false;
    }
  }

  bool erase_n(int n)
  {
    if (n > 0) {
      for (int i = 0; i < n; ++i) {
        if (!erase_front()) {
          return false;
        }
      }
   /* } else if (n < 0) {
      n = 0 - n;

      for (int i = 0; i < n; ++i) {
        if (!erase_back()) {
          return false;
        }
      }*/
    }

    return true;
  }

  bool erase_front()
  {
    if (list_.empty()) {
      return false;
    } else {
      unordered_set_.erase(list_.front());
      list_.erase_and_dispose(list_.begin(), delete_disposer<ID_Attribute>());
      return true;
    }
  }

  void clear()
  {
    while (erase_front()) {}
  }
  ~Cache(){clear();}

private:
  list_t     list_;
  typename unordered_set_t::bucket_type buckets_[size];
  unordered_set_t  unordered_set_;
};
template<class Key, class T, int size  >
std::ostream& operator<<(std::ostream&os, const Cache<Key, T, size>&cache)
{
  typedef ID_Attr<Key, T> ID_Attribute;
  typename Cache<Key, T, size>::list_t::const_iterator it = cache.list_.begin();
  typename Cache<Key, T, size>::list_t::const_iterator ite = cache.list_.end();

  for (; it != ite; ++it) {
    std::cout << *it;
  }

  return os;
}
}
#endif
