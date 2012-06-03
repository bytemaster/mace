#include <mace/cmt/thread.hpp>

int hello( const std::string& s ) { return s.size(); }

using namespace boost::chrono;
namespace cmt = mace::cmt;

// 1.87e6
int main( int argc, char** argv ) {
    system_clock::time_point start = system_clock::now();
    cmt::thread::current().set_name("main");
    elog( "Creating thread" );
    cmt::thread* t = cmt::thread::create("bench");
    elog( "created thread: %1%", t );
    int sum = 0;
    for( uint32_t i = 0; i < 100000000; ++i )  {
        sum += t->async( bind(hello, std::string("world") ) ).wait();
	if( i && 0 == i % 100000 )
	    slog( "%1% calls/sec", 
	      (double(i)/((system_clock::now() - start).count()/1000000000.0)) );
    }
    slog( "%1% calls/sec", 
      (100000000.0/((system_clock::now() - start).count()/1000000000.0)) );
    t->quit();
}
