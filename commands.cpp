#include <boost/foreach.hpp>
#include <iostream>
#include "minimemcache.h"
#include "commands.h"
void minimemcache::introduction()
{
	std::cout<<"Welcome to MiniMemCached. \n"
		<<"MiniMemCached is a mini memcache server. Have fun! \n"
		<<"Copyright (c) 2013 by Jiayuehua. \n" 
		<<std::endl ;
}
const char* minimemcache::help_message()
{
	static const char * help_message=
        "ls               : list all items\n"
        "clear            : erase all items in the cache\n"
        "eraseold         : erase oldest item in the cache\n"
        "erasen number    : number indicate the num of items to erase. negative means erase new.\n"
        "erase  Id        : erase item with key Id.\n"
        "set id attribute : insert <id,attribute> pair in the cache, id is the key of item.\n"
        "get    id        : print attribute of id.\n";
	return help_message;
}
int main()
{
	//minimemcache::cmdline_use<int, yasli::string, 901>::initial_hooks();
	minimemcache::Commands<int, int, 901>::initial_hooks();
	minimemcache::Commands<int, int, 901>::handle_request();
	return 0;
}
