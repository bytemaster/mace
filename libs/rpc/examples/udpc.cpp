#include <mace/rpc/raw/udp/connection.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <mace/rpc/raw/raw_io.hpp>
#include <mace/rpc/udp/client.hpp>

namespace rpc = mace::rpc;

class calculator {
  public:
    double add( double a, double b )const { slog( "%1% + %2% = %3%", a, b, (a+b) ); return a+b; }
    double sub( double a, double b )const { slog( "%1% - %2% = %3%", a, b, (a-b) ); return a-b; }
};
MACE_STUB( calculator, (add)(sub) )

int main( int argc, char** argv ) {
 try {
     if( argc <= 1 ) {
       std::cerr << "Usage: "<<argv[0]<<" IP PORT\n"; return -1;
     }
     mace::rpc::udp::client<calculator> c;
     c.connect( rpc::udp::endpoint(boost::asio::ip::address_v4::from_string( argv[1]), 
                boost::lexical_cast<uint16_t>(argv[2]) ) );
     slog( "result: %1%", c->add( 5, 6 ).wait() );
     slog( "result: %1%", c->add( 6, 7 ).wait() );
     slog( "result: %1%", c->add( 7, 8 ).wait() );
     return 0;
  } catch ( const boost::exception& e ) {
    elog( "%1%", boost::diagnostic_information(e) );
  } catch ( const std::exception& e ) {
    elog( "%1%", boost::diagnostic_information(e) );
  }
};
