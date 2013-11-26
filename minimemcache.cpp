#include "minimemcached.h"
#include <boost/foreach.hpp>
#include <iostream>
void introduction()
{
	std::cout<<"Welcome to MiniMemCached. \n"
		<<"MiniMemCached is a mini memcache server. Have fun! \n"
		<<"Copyright (c) 2013 by Jiayuehua. \n" 
		<<std::endl ;
}
int main()
{
	cmdline_use<int, yasli::string, 901>::initial_hooks();
	cmdline_use<int, yasli::string, 901>::handle_request();
	return 0;
}
