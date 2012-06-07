#include <mace/cmt/mutex.hpp>
#include <mace/cmt/thread.hpp>

void print() {
  std::cerr<<"A\n";
  mace::cmt::usleep(1000000);
  std::cerr<<"B\n";
  mace::cmt::usleep(1000000);
  std::cerr<<"C\n";
  mace::cmt::usleep(1000000);
  std::cerr<<"\n";
};


mace::cmt::mutex m;
void print_mutex() {
  boost::unique_lock<mace::cmt::mutex> lock(m);
  std::cerr<<"A\n";
  mace::cmt::usleep(100000);
  std::cerr<<"B\n";
  mace::cmt::usleep(100000);
  std::cerr<<"C\n";
  mace::cmt::usleep(100000);
  std::cerr<<"\n";
};


int main( int argc, char** argv ) {
  slog( "Start no mutex test" );
  auto r1 = mace::cmt::async(print);
  auto r2 = mace::cmt::async(print);
  auto r3 = mace::cmt::async(print);

  slog("wait for result...");
  r1.wait(); r2.wait(); r3.wait(); 

  slog( "Start 10 mutex test" );
  std::vector<mace::cmt::future<void> > r;
  for( uint32_t i = 0; i < 10; ++i )
    r.push_back( mace::cmt::async(print_mutex) );

  slog( "only wait for 5 of them" );
  for( uint32_t i = 0; i < 5; ++i )
    r[i].wait();

  slog( "quiting" );
  mace::cmt::quit();
  return 0;
}
