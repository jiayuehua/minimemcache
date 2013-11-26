minimemcache
============

A memcached like memory cache with less code and high performance.

Dependency library  : Boost 1.53 and loki 0.17.

Discription:
flex_string.h flex_string_shell.h is borrowed from loki library, with many changes. In order to compile minimemcache, you should replace the loki's coresponding files with those two file. Or you can "s/yasli::string/std::string/g" and add one line of code "#include "string""  in the minimemcache.cpp

Now it is just can be used as a library in single thread. In the future minimemcache will be used as a service of ACE server and support multithread operation.

Commands:

    ls               : list all items.
    
    clear            : erase all items in the cache.
    
    eraseold         : erase oldest item in the cache.
    
    erasen number    : number indicate the num of items to erase. negative means erase new.
    
    erase  Id        : erase item with key Id.
    
    set id attribute : insert <id,attribute> pair in the cache, id is the key of item.
    
    get    id        : print attribute of id.

author jiayuehua gracedancer@163.com

