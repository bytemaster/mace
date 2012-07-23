#include <mace/rpc/json/io.hpp>
#include <mace/sigar.hpp>
#include <mace/cmt/thread.hpp>

int main( int argc, char** argv ) {
  if( argc > 1 )
  std::cout<<mace::rpc::json::to_pretty_string( mace::system_status(argv[1]) )<<std::endl;
  else
  std::cout<<mace::rpc::json::to_pretty_string( mace::system_status() )<<std::endl;


  mace::sigar s;
  while( true ) {
  std::cout<<"CPU %:" <<s.percent_cpu_usage()<<"  Load Avg: "<<s.load_average()<<std::endl;
  mace::cmt::usleep(1000000);
  }

  return 0;
}
