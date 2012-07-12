#include <mace/rpc/json/process/server.hpp>
#include <mace/cmt/signals.hpp>

class test_fixture {
  public:
    std::string hello( std::string w ) { return "Hi, " + w; }
};

MACE_STUB( test_fixture, (hello) )

int main( int argc, char** argv ) {
  try {
      std::cerr<<"Child Cerr\n";
      auto tf( std::make_shared<test_fixture>() );
      mace::rpc::json::process::server<test_fixture> s( tf, std::cin, std::cout );
      mace::cmt::wait( s.closed );
      std::cerr<<"Done\n";
      return 0;
  } catch ( ... ) {
    elog( "%1%", boost::current_exception_diagnostic_information() );
    return -1;
  }
}
