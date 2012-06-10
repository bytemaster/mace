#include <mace/cmt/thread.hpp>
#include <mace/cmt/bind.hpp>

int hello( const std::vector<char>& s ) { return s.size(); }

using namespace boost::chrono;
namespace cmt = mace::cmt;

int main( int argc, char** argv ) {
    system_clock::time_point start = system_clock::now();
    cmt::thread::current().set_name("main");
    elog( "Creating thread" );
    cmt::thread* t = cmt::thread::create("bench");
    elog( "created thread: %1%", t );
    int sum = 0;
    std::string s("hello");
    std::string w("world");
    s += w;
    std::vector<char> data(1);
    for( uint32_t i = 0; i < 100000000; ++i )  {
        sum += t->async( mace::cmt::bind(&hello, mace::cmt::copy(data) ) ).wait();
	if( i && 0 == i % 100000 ) {
	    slog( "%1% calls/sec", 
	      (double(100000)/((system_clock::now() - start).count()/1000000000.0)) );
      start = system_clock::now();
  }
    }
    slog( "%1% calls/sec", 
      (100000000.0/((system_clock::now() - start).count()/1000000000.0)) );
    t->quit();
}
