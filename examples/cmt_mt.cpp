#include <mace/cmt/thread.hpp>

int hello( const std::string& s ) { return s.size(); }

namespace cmt = mace::cmt;

int main( int argc, char** argv ) {
  try{ 
   cmt::thread::current().set_name("main");
   cmt::thread* t1 = cmt::thread::create( "t1" );
   cmt::thread* t2 = cmt::thread::create( "t2" );

   cmt::future<int> f1 = t1->async( bind(hello,"world1") );
   auto f2 = t2->async( bind(hello,"world2") );
   auto f3 = cmt::async<int>( bind(hello,"world3") );

   // world3 is processed in current thread, while waiting on f1 & f2
   std::cerr<<( f1.wait() + f2.wait() + f3.wait() ) << std::endl;

   t1->quit(); // cancel any tasks and join thread
   t2->quit(); // cancel any tasks and join thread

   cmt::thread::current().quit();
 } catch ( ... ) {
    elog( "Caught exception" );
    return 1; 
 }
 return 0;
}
