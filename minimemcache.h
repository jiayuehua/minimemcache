#ifndef MINIMEMCACHE_H
#define MINIMEMCACHE_H
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <boost/functional/hash.hpp>
#include <loki/Singleton.h>
#include <loki/AssocVector.h>

#include <iosfwd>
#include <loki/flex/flex_string.h>

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

template <class TK, class V>
class ID_Attr
{
	std::pair<TK, V> id_name_;
public:
	explicit ID_Attr(TK data, const V& s = V()  ):id_name_(data, s){
	}
	void set(int id, const yasli::string&s)  {  id_name_ = std::make_pair(id, s); }
	const yasli::string & get() const  { return id_name_.second;    }

	//This class can be inserted in an intrusive list
	intrusive::list_member_hook<>   list_hook_;

	//This class can be inserted in an intrusive unordered_set
	intrusive::unordered_set_member_hook<>   unordered_set_hook_;

	//Comparison operators
	friend bool operator==(const ID_Attr<TK, V> &a, const ID_Attr<TK,V> &b)
	{  return a.id_name_.first == a.id_name_.first; }

	friend std::ostream& print_IDAttr(std::ostream&os, const ID_Attr<TK, V> &b);

	friend bool operator!=(const ID_Attr<TK, V> &a, const ID_Attr<TK,V> &b)
	{  return a.id_name_.first != b.id_name_.first; }

	//The hash function
	friend std::size_t hash_value(const ID_Attr<TK,V> &i)
	{  return boost::hash<TK>()(i.id_name_.first);  }
};

template <class TK, class V>
	 std::ostream& print_IDAttr(std::ostream&os, const ID_Attr<TK, V> &b) 
	{
		os<<b.id_name_.first<<"    "<<b.id_name_.second<<"\n";
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
	friend std::ostream& print_cache(std::ostream&os, const Cache<ID, Attribute, size>&cache);
public:
	Cache():unordered_set_( unordered_set_t::bucket_traits(buckets_, size)){}
	std::pair<bool, yasli::string> get(int id)
	{
		unordered_set_t::iterator unordered_set_it = unordered_set_.find( ID_Attribute(id) );
		if (unordered_set_it == unordered_set_.end())
		{
			return std::make_pair(false, yasli::string());
		}
		else
		{
			list_t::iterator l_it = list_.iterator_to(*unordered_set_it);
			list_.erase(l_it);
			list_.push_back(*unordered_set_it);
			return std::make_pair(true, unordered_set_it -> get( ) );
		}
	}

	void set(int id, const yasli::string& s)
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

	bool erase(int id)
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
std::ostream& print_cache(std::ostream&os, const Cache<ID, Attribute, size>&cache)
{
	typedef ID_Attr<ID, Attribute> ID_Attribute;
	BOOST_FOREACH(ID_Attribute s, cache.list_)
	{
		print_IDAttr(s);
	}
	return os;
}

template<class ID, class Attribute, int size>
std::istream& ls(std::istream & is, Cache<ID, Attribute, size>& cache)
{
	//print_cache(std::cout, cache);
	return is>>ignoreLine; 
}

template<class ID, class Attribute, int size>
std::istream& set(std::istream &is, Cache<ID, Attribute, size>& cache)
{
	yasli::string name;
	int id = 0;
	is>>id >>name >>ignoreLine;
	cache.set(id, name);
	return is;
}

template<class ID, class Attribute, int size>
std::istream& erase(std::istream &is, Cache<ID, Attribute, size>& cache)
{
	int id = 0;
	is>>id>>ignoreLine;
	cache.erase(id);
	return is;
}

template<class ID, class Attribute, int size>
std::istream& erase_n(std::istream &is, Cache<ID, Attribute, size>& cache)
{
	int n = 0;
	is>>n;
	cache.erase_n(n);
	return is>>ignoreLine;
}

template<class ID, class Attribute, int size>
std::istream& get(std::istream &is, Cache<ID, Attribute, size>& cache)
{
	int id = 0;
	is>>id;
	if (cache.get(id).first)
	{
		std::cout<<cache.get(id).second<<std::endl;
	}
	else
	{
		std::cerr<<id<<" value don't exist!\n";
	}
	return is>>ignoreLine;
}

template<class ID, class Attribute, int size>
std::istream& erase_old(std::istream &is, Cache<ID, Attribute, size>& cache)
{
	cache.erase_front();
	return is>>ignoreLine;
}

template<class ID, class Attribute, int size>
std::istream& clear(std::istream &is, Cache<ID, Attribute, size>& cache)
{
	cache.clear();
	return is>>ignoreLine;
}


template <class ID, class Attribute, int size>
class cmdline_use{
	typedef std::istream& (*Cmd_Func)(std::istream &is, Cache<ID, Attribute, size>& cache);
	typedef Loki::SingletonHolder<Loki::AssocVector<yasli::string, Cmd_Func> >Cmd_Funcs;
	typedef Loki::SingletonHolder<Cache<ID, Attribute, size> > UniCache;
	static	std::istream& invalid_cmd(std::istream &is, Cache<ID, Attribute, size>& cache)
	{
		std::cout<<"Command should be: ";
		for (Cmd_Funcs::ObjectType::iterator i = Cmd_Funcs::Instance().begin(); i!= Cmd_Funcs::Instance().end(); ++i)
		{
			std::cout<<i->first<<" ";
		}
		std::cout<<"!!\n";
		std::cout<<std::endl;
		return is>>ignoreLine;
	}
public:
	static void initial_hooks() 
	{
		Cmd_Funcs::Instance().insert(std::make_pair("erase_old" , &erase_old <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("get" 		, &get       <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("set"		, &::set     <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("ls"		, &ls        <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("erase"     , &erase     <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("erasen"    , &erase_n   <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("clear"     , &clear     <ID, Attribute, size> ) );
		introduction();
	}

	static	void handle_request() 
	{
		yasli::string cmd;
		std::cout<<"> "<<std::flush;
		while(std::cin>>cmd)
		{
			Cmd_Funcs::ObjectType::iterator it = Cmd_Funcs::Instance().find(cmd);
			if (it != Cmd_Funcs::Instance().end())
			{
				(*it->second)(std::cin, UniCache::Instance());
			}
			else
			{
				invalid_cmd(std::cin, UniCache::Instance());
			}
			std::cout<<"> "<<std::flush;
		}
	}
};

void introduction();

#endif