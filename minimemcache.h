#ifndef MINIMEMCACHE_H
#define MINIMEMCACHE_H
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <boost/functional/hash.hpp>

#include <iostream>
namespace minimemcache{
	template <class ID , class Attribute, int size>
	class Cache;
	template<class ID, class Attribute, int size  >
	std::ostream& operator<<(std::ostream&os, const Cache<ID, Attribute, size>&cache);
template <class charT, class traits>
inline
std::basic_istream<charT,traits>&
ignoreLine (std::basic_istream<charT,traits>& strm)
{
	// skip until end-of-line
	strm.ignore(std::numeric_limits<int>::max(),strm.widen('\n'));

	// return stream for concatenation
	return strm;
}

//using namespace boost::intrusive;
namespace intrusive = boost::intrusive;
template <class ID, class V>
class ID_Attr;
template <class ID, class V>
std::ostream& operator<<(std::ostream&os, const ID_Attr<ID, V> &b);
template <class ID, class V>
class ID_Attr
{
	//friend std::ostream& operator<<(std::ostream&os, const ID_Attr<ID, V> &b);
	std::pair<ID, V> id_name_;
	
public:
	void print(std::ostream & os)const
	{
		os<<id_name_.first<<"    "<<id_name_.second<<"\n";
	}
	explicit ID_Attr(ID data, const V& s = V()  ):id_name_(data, s){
	}
	void set(ID id, const V&s)  {  id_name_ = std::make_pair(id, s); }
	const V & get() const  { return id_name_.second;    }

	//This class can be inserted in an intrusive list
	intrusive::list_member_hook<>   list_hook_;

	//This class can be inserted in an intrusive unordered_set
	intrusive::unordered_set_member_hook<>   unordered_set_hook_;

	//Comparison operators
	friend bool operator==(const ID_Attr<ID, V> &a, const ID_Attr<ID,V> &b)
	{  return a.id_name_.first == a.id_name_.first; }


	friend bool operator!=(const ID_Attr<ID, V> &a, const ID_Attr<ID,V> &b)
	{  return a.id_name_.first != b.id_name_.first; }

	//The hash function
	friend std::size_t hash_value(const ID_Attr<ID,V> &i)
	{  return boost::hash<ID>()(i.id_name_.first);  }
};

template <class ID, class V>
std::ostream& operator<<(std::ostream&os, const ID_Attr<ID, V> &b) 
{
	//os<<b.id_name_.first<<"    "<<b.id_name_.second<<"\n";
	b.print(os);
	return os;
}

template <class T>
struct delete_disposer
{
	void operator()(T *delete_this)
	{  delete delete_this;  }
};

template<class ID, class Attribute ,int size = 901>
class Cache
{
	//friend std::ostream& operator<<(std::ostream&os, const Cache<ID, Attribute, size>&cache);
public:
	void print(std::ostream & os)const
 {
	typedef ID_Attr<ID, Attribute> ID_Attribute;
	BOOST_FOREACH(ID_Attribute s, list_)
	{
		os<<(s);
	}
}

private:
	//Definition of the intrusive list that will hold ID_Attribute
	typedef ID_Attr<ID, Attribute> ID_Attribute;
	typedef intrusive::member_hook<ID_Attribute, intrusive::list_member_hook<>
	, &ID_Attribute::list_hook_> MemberListOption;
	typedef intrusive::list<ID_Attribute, MemberListOption> list_t;

	//Definition of the intrusive unordered_set that will hold ID_Attribute
	typedef intrusive::member_hook
	< ID_Attribute, intrusive::unordered_set_member_hook<>
	, &ID_Attribute::unordered_set_hook_> MemberUsetOption;
	typedef intrusive::unordered_set
	< ID_Attribute, MemberUsetOption> unordered_set_t;
public:
	Cache():unordered_set_( unordered_set_t::bucket_traits(buckets_, size)){}
	std::pair<bool, Attribute> get(int id)
	{
		unordered_set_t::iterator unordered_set_it = unordered_set_.find( ID_Attribute(id) );
		if (unordered_set_it == unordered_set_.end())
		{
			return std::make_pair(false, Attribute());
		}
		else
		{
			list_t::iterator l_it = list_.iterator_to(*unordered_set_it);
			list_.erase(l_it);
			list_.push_back(*unordered_set_it);
			return std::make_pair(true, unordered_set_it -> get( ) );
		}
	}

	void set(ID id, const Attribute& s)
	{
		unordered_set_t::iterator unordered_set_it = unordered_set_.find( ID_Attribute(id) );
		if (unordered_set_it != unordered_set_.end())
		{
			erase(id);
		}
		ID_Attribute * pnode = new ID_Attribute(id, s);
		list_.push_back(*pnode);
		unordered_set_.insert(*pnode);
	}

	bool erase(ID id)
	{
		if (list_.empty())
		{
			return false;
		}
		unordered_set_t::iterator unordered_set_it = unordered_set_.find( ID_Attribute(id) );
		if( unordered_set_it != unordered_set_.end())
		{
			unordered_set_.erase(*unordered_set_it);
			list_.erase_and_dispose(list_.iterator_to(*unordered_set_it),delete_disposer<ID_Attribute>());
			return true;
		}
		else
		{
			return false;
		}
	}

	bool erase_n(int n)
	{
		if (n>0)
		{
			for (int i = 0; i<n; ++i)
			{
				if (!erase_front())
				{
					return false;
				}
			}
		}
		else if (n <0)
		{
			n = 0 - n;
			for (int i = 0; i<n; ++i)
			{
				if (!erase_back())
				{
					return false;
				}
			}
		}
		return true;
	}

	bool erase_back()
	{
		if (list_.empty())
		{
			return false;
		}
		else
		{
			unordered_set_.erase(list_.back());
			list_t::iterator i = list_.end();
			--i;
			list_.erase_and_dispose(i,delete_disposer<ID_Attribute>());
			return true;
		}
	}

	bool erase_front()
	{
		if (list_.empty())
		{
			return false;
		}
		else
		{
			unordered_set_.erase(list_.front());
			list_.erase_and_dispose(list_.begin(),delete_disposer<ID_Attribute>());
			return true;
		}
	}

	void clear()
	{
		while (erase_front()){}
	}

private:
	list_t     list_;
	typename unordered_set_t::bucket_type buckets_[size];
	unordered_set_t  unordered_set_;
};

template<class ID, class Attribute, int size  >
std::ostream& operator<<(std::ostream&os, const Cache<ID, Attribute, size>&cache)
{
	cache.print(os);
	return os;
}
template<class ID, class Attribute, int size>
std::istream& ls(std::istream & is, Cache<ID, Attribute, size>& cache)
{
	std::cout<<cache;
	return is>>ignoreLine; 
}
}

#endif