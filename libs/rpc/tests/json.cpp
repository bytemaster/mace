#include <mace/rpc/json/file_io.hpp>
int main( int argc, char** argv ) {
  mace::rpc::value v = mace::rpc::json::from_file( argv[1] );
  std::cout<< mace::rpc::json::to_pretty_string( v ) << std::endl;
  std::string s = mace::rpc::json::to_pretty_string( v );
  auto v2 = mace::rpc::json::read_value( s.begin(), s.end() );
  std::cout<< std::string(v2.begin(),v2.end()) << std::endl;
  return 0;
}
