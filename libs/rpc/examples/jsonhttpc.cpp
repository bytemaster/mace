#include <mace/rpc/json/client.hpp>
#include "cli.hpp"
#include <boost/exception/diagnostic_information.hpp>
#include <mace/rpc/json/http_client.hpp>
#include "bitcoin.hpp"

using namespace mace::rpc::json;
int main( int argc, char** argv ) {
  if( argc != 3 ) { 
    std::cerr<<"Usage "<<argv[0]<<" URL USER:PASSWORD\n"; 
    return -1; 
  }
  try {
    json::http_client<bitcoin::client> bcd( argv[1] );
    bcd.set_header( "Authorization", "Basic "  + mace::rpc::base64_encode(argv[2]) );

    cli  m_cli(bcd);
    m_cli.start();

    mace::cmt::exec();
  } catch (const  boost::exception& e ) {
    elog( "%1%", boost::diagnostic_information(e) );
  } catch (const  std::exception& e ) {
    elog( "%1%", boost::diagnostic_information(e) );
  } catch ( ... ) {
    elog( "Caught unknown exception" );
  }
  return 0;
}
