#ifndef CMDLINE_CLIENT
#define CMDLINE_CLIENT
#include <loki/Singleton.h>
#include <loki/AssocVector.h>
#include <loki/flex/flex_string.h>
#include "minimemcache.h"
namespace minimemcache{

template<class ID, class Attribute, int size>
std::istream& setItem(std::istream &is, Cache<ID, Attribute, size>& cache)
{
	ID id ;
	Attribute attr ;
	is>>id >>attr >>ignoreLine;
	cache.set(id, attr);
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

template<class ID, class Attribute, int size>
std::istream& help(std::istream &is, Cache<ID, Attribute, size>& cache)
{
	std::cout<<help_message();
	return is>>ignoreLine;
}
const char* help_message();
template <class ID, class Attribute, int size>
class Commands{
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
		//std::cout<<help_message();
		std::cout<<".\n";
		std::cout<<std::endl;
		return is>>ignoreLine;
	}
public:
	static void initial_hooks() 
	{
		Cmd_Funcs::Instance().insert(std::make_pair("erase_old" , &erase_old <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("get" 		, &get       <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("set"		, &setItem   <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("ls"		, &ls        <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("erase"     , &erase     <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("erasen"    , &erase_n   <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("clear"     , &clear     <ID, Attribute, size> ) );
		Cmd_Funcs::Instance().insert(std::make_pair("help"      , &help      <ID, Attribute, size> ) );
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
}
#endif