minimemcache
============

A memcached like memory cache with less code and high performance.

To compile the minicache. Boost 1.53 and loki 0.17 library is needed.

Source file discription:
flex_string.h flex_string_shell.h is borrowed from loki, with some changes. In order to compile minimemcache, you should replace the loki's coresponding files with those two file. Or you can "s/yasli::string/std::string/g" and add one line of code "#include <string>"  in the minimemcache.cpp

Now it is just can be a part of library in single thread. In the future minimemcache will be used as a type of ACE server.


