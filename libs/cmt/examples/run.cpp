#include <mace/cmt/process.hpp>

int main( int argc, char** argv ) {
  if( argc < 2 ) {
    std::cerr<<"usage: "<<argv[0]<<" command args...\n";
    return -1;
  }

  std::vector<std::string> args;
  for( uint32_t i = 2; i < argc; ++i )
    args.push_back(argv[i]);

  mace::cmt::process p = mace::cmt::process::exec( argv[1], std::move(args) );

  std::string line;
  std::getline( p.out_stream(), line );
  while( p.out_stream() ) {
    std::cout<<line<<std::endl;
    std::getline( p.out_stream(), line );
  }
  return p.result();
}
