#include <mace/cmt/thread.hpp>

int hello( const std::string& s ) { return s.size(); }

using namespace boost::chrono;
namespace cmt = mace::cmt;

int main( int argc, char** argv ) {
    system_clock::time_point start = system_clock::now();
    cmt::thread::current().set_name("main");
    elog( "Creating thread" );
    cmt::thread* t = cmt::thread::create("bench");
    elog( "created thread: %1%", t );
    int sum = 0;
    for( uint32_t i = 0; i < 10000000; ++i )  {
        sum += t->async<int>( bind(hello, std::string("world") ) ).wait();
    }
    system_clock::time_point end = system_clock::now();
    slog( "%1% calls/sec", 
      (10000000.0/((system_clock::now() - start).count()/1000000000.0)) );
    t->quit();
}
