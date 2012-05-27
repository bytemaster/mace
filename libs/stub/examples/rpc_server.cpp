#include <boost/exception/diagnostic_information.hpp>
#include "calculator.hpp"
#include "rpc.hpp"
#include <boost/lexical_cast.hpp>

using namespace mace;
//! [Define Class]
class CalculatorServer {
  public:
    std::string name()const            { return "CalculatorServer"; }
    int    exit()                      { ::exit(0);                  }
    double add( double v, double v2 )  { return m_result += v + v2;  }
    double sub( double v, double v2 )  { return m_result -= v - v2;  }
  private:
    double m_result;
};
//! [Define Class]

int main( int argc, char** argv ) {
  if( argc <= 1 ) {
    std::cerr << "Usage: rpc_server PORT\n";
    return -1;
  }
  using namespace boost;
  try {
//! [Assign Stub]
    stub::ptr<calculator> calc( boost::make_shared<CalculatorServer>() );
//! [Assign Stub]
    stub::rpc_server server( calc );
    server.listen( lexical_cast<uint16_t>(argv[1]) );
  } catch ( const boost::exception& e ) {
    std::cerr << boost::diagnostic_information(e) << std::endl;
  }
  return 0;
}


