#include <mace/cmt/thread.hpp>

int hello( int in ){ return in; }

int main( int argc, char** argv ) {
    boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();
    int sum = 0;
    for( uint32_t i = 0; i < 1000000; ++i ) 
        sum += mace::cmt::async<int>( std::bind(hello, i) ).wait();
    boost::chrono::system_clock::time_point end = boost::chrono::system_clock::now();
    slog( "%1% calls/sec", 
      (1000000.0/((end - start).count()/1000000000.0)) );
}
