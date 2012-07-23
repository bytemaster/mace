#include <mace/rpc/json/process_server.hpp>
#include <mace/cmt/signals.hpp>
#include <mace/rpc/error.hpp>
#include <mace/rpc/connection_base.ipp>

class test_fixture {
  public:
    std::string hello( std::string w ) { /*MACE_RPC_THROW( "Test" ); throw std::runtime_error( "Hello Exception" );*/ return "Hi, " + w; }
    std::vector<double> vec( const std::vector<double>& v ) { std::cerr.flush(); return v; }
};

MACE_STUB( test_fixture, (hello)(vec) )

int main( int argc, char** argv ) {
  try {
      std::cerr<<"Child Cerr\n";
      auto tf =  std::make_shared<test_fixture>();
      mace::rpc::json::process::server<test_fixture> s( tf, std::cin, std::cout, "cin" );
      mace::cmt::wait( s.closed );
      std::cerr<<"Done\n";
      return 0;
  } catch ( ... ) {
    elog( "%1%", boost::current_exception_diagnostic_information() );
    return -1;
  }
}
