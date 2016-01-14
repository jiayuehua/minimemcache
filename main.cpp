#include <iostream>
#include "minimemcache.h"
int main()
{
  minimemcache::Cache<int, int> c;
  c.insert(1,2);
  c.insert(5,6);
  c.insert(8,7);
  c[1] = 90;
  std::cout<<c;
  std::cout<<std::endl;
  c.erase_front();
  std::cout<<c;
}