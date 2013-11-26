minimemcache
============

A memcached like memory cache with less code and high performance.

Depend library  : Boost 1.53 and loki 0.17.

Discription:
flex_string.h flex_string_shell.h is borrowed from loki, with many changes. In order to compile minimemcache, you should replace the loki's coresponding files with those two file. Or you can "s/yasli::string/std::string/g" and add one line of code "#include "string""  in the minimemcache.cpp

Now it is just can be used as a library in single thread. In the future minimemcache will be used as a service of ACE server.

author jiayuehua gracedancer@163.com

