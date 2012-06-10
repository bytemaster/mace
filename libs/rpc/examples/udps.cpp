#include <mace/rpc/raw/udp/server.hpp>
#include <iostream>
namespace rpc = mace::rpc;
namespace ip  = boost::asio::ip;

class calculator {
  public:
    double add( double a, double b )const { slog( "%1% + %2% = %3%", a, b, (a+b) ); return a+b; }
    double sub( double a, double b )const { slog( "%1% - %2% = %3%", a, b, (a-b) ); return a-b; }
};
MACE_STUB( calculator, (add)(sub) )

int main( int argc, char** argv ) {
  if( argc <= 1 ) {
     std::cerr << "Usage: "<<argv[0]<<" PORT\n"; return -1;
  }
  try {
    auto s = std::make_shared<calculator>();
    mace::rpc::raw::udp::server<calculator>  serv( s, rpc::udp::endpoint( ip::address(), boost::lexical_cast<int>(argv[1]) ) );
    mace::cmt::exec();
  } catch ( ... ) {
    std::cerr << boost::current_exception_diagnostic_information() << std::endl;
  }
};
