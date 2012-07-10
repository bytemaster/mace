#include <mace/rpc/json/pipe/connection.hpp>
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
   
   mace::stub::ptr<test_fixture> serv( std::make_shared<test_fixture>() );

   typedef mace::rpc::json::pipe::connection<> connection;
   auto c = connection::ptr( new connection( std::cin, std::cout ) );

   mace::stub::visit( serv, connection::add_interface_visitor<test_fixture>( *c, serv ) );

   mace::cmt::async( print_err );

   mace::cmt::wait( c->closed );

   return 6;
}
