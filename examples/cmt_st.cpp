#include <mace/cmt/thread.hpp>
using boost::chrono::system_clock;
namespace cmt = mace::cmt;

int hello( int in ){ return in; }

int main( int argc, char** argv ) {
    system_clock::time_point start = system_clock::now();
    int sum = 0;
    for( uint32_t i = 0; i < 100000000; ++i ) {
        sum += cmt::async( std::bind(hello, i) ).wait();
        if( i % 1000000 == 0 ) {
            system_clock::time_point end = system_clock::now();
            slog( "%1% calls/sec", (i/((end - start).count()/1000000000.0)) );
        }
    }
}
