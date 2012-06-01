#include <mace/cmt/thread.hpp>
#include <mace/cmt/signals.hpp>

namespace cmt = mace::cmt;

boost::signal<void(std::string)> test_signal;

void delay() {
    cmt::usleep(2000000);
    test_signal("hello world!");
}

int main( int argc, char** argv ) {
     std::cerr<< "Delay for 2 seconds...\n";
     cmt::async( delay );
     std::cerr<< cmt::wait(test_signal) << std::endl;
     return 0;
}
