#include <mace/rpc/json/process_server.hpp>
#include <fstream>
#include <mace/cmt/signals.hpp>

class test_fixture {
  public:
    std::string hello( std::string w ) { return "Hi, " + w; }
};

void print_err(){
  while( true ) {
    std::cerr<<"Error I'm Alive!\n";
    std::cerr.flush();
    mace::cmt::usleep( 100000 ); 
  }
}
void print_std(){
  while( true ) {
    std::cout<<"Std I'm Alive!\n";
    std::cout.flush();
    mace::cmt::usleep( 300000 ); 
  }

}


MACE_STUB( test_fixture, (hello) )

int main( int argc, char** argv ) {
  mace::cmt::thread::current().set_name( "runner_main" );
  try {
      mace::cmt::async( print_err );

      auto tf( std::make_shared<test_fixture>() );
      mace::rpc::json::process::server<test_fixture> s( tf, std::cin, std::cout, "cin" );
      mace::cmt::wait( s.closed );
      return 0;
  } catch ( ... ) {
    elog( "%1%", boost::current_exception_diagnostic_information() );
    return -1;
  }
  /* 
   mace::stub::ptr<test_fixture> serv( std::make_shared<test_fixture>() );

   typedef mace::rpc::json::pipe::connection<> connection;
   auto c = connection::ptr( new connection( std::cin, std::cout ) );

   mace::stub::visit( serv, connection::add_interface_visitor<test_fixture>( *c, serv ) );


   mace::cmt::wait( c->closed );
   */

   return 6;
}
