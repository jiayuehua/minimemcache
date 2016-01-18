//#include <iostream>
//#include <iterator>
//#include <set>
//#include <utility>
//#include <stdint.h>
//#include "minimemcache.h"
//int main()
//{
//  std::set<int> s;
//  s.insert(1 );
//  s.insert(6 );
//  s.insert(3 );
//  s.insert(8 );
//  std::ostream_iterator<int> oi(std::cout, " ");
//  std::set<int>::iterator l  = s.lower_bound(2);
//  std::set<int>::iterator r = s.upper_bound(7);
//  std::copy(l, r, oi );
//
// /* minimemcache::Cache<int, int> c;
//  c.insert(1,2);
//  c.insert(5,6);
//  c.insert(8,7);
//  c[1] = 90;
//  std::cout<<c;
//  std::cout<<std::endl;
//  c.erase_front();
//  std::cout<<c;*/
//}

#include <boost/intrusive/list.hpp>
#include <cassert>

using namespace boost::intrusive;
struct tag1;
struct tag2;
typedef list_base_hook<link_mode<auto_unlink>, tag<tag1> > auto_unlink_hook;
typedef list_base_hook<link_mode<auto_unlink>, tag<tag2> > auto_unlink_hook2;

class MyClass : public auto_unlink_hook,public auto_unlink_hook2
               //This hook removes the node in the destructor
{
   int int_;

   public:
   MyClass(int i = 0)   :  int_(i)  {}
   void unlink()     {  auto_unlink_hook::unlink(); auto_unlink_hook2::unlink();  }
   bool is_linked_list1()  {  return auto_unlink_hook::is_linked();  }
   bool is_linked_list2()  {  return auto_unlink_hook2::is_linked();  }
};

//Define a list that will store values using the base hook
//The list can't have constant-time size!
typedef list< MyClass, base_hook<auto_unlink_hook>,  constant_time_size<false> > List;
typedef list< MyClass, base_hook<auto_unlink_hook2>, constant_time_size<false> > List2;

int main()
{
   //Create the list
   List l;
   {
      //Create myclass and check it's linked
      MyClass myclass;
      assert(myclass.is_linked_list1() == false);

      //Insert the object
      l.push_back(myclass);

      //Check that we have inserted the object
      assert(l.empty() == false);
      assert(&l.front() == &myclass);
      assert(myclass.is_linked_list1() == true);
      assert(myclass.is_linked_list2() == false);

      //Now myclass' destructor will unlink it
      //automatically
   }

   //Check auto-unlink has been executed
   assert(l.empty() == true);

   {
      ////Now test the unlink() function

      ////Create myclass and check it's linked
      //MyClass myclass;
      //assert(myclass.is_linked() == false);

      ////Insert the object
      //l.push_back(myclass);

      ////Check that we have inserted the object
      //assert(l.empty() == false);
      //assert(&l.front() == &myclass);
      //assert(myclass.is_linked() == true);

      ////Now unlink the node
      //myclass.unlink();

      ////Check auto-unlink has been executed
      //assert(l.empty() == true);
   }
   return 0;
}