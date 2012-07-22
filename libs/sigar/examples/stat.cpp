#include <mace/rpc/json/io.hpp>
#include <mace/sigar.hpp>

int main( int argc, char** argv ) {
  if( argc > 1 )
  std::cout<<mace::rpc::json::to_pretty_string( mace::system_status(argv[1]) )<<std::endl;
  else
  std::cout<<mace::rpc::json::to_pretty_string( mace::system_status() )<<std::endl;
  return 0;
}
