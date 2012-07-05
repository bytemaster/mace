#include <mace/rpc/json/pipe/connection.hpp>

class test_fixture {
  public:
    std::string hello( std::string w ) { return "Hi, " + w; }
};


MACE_STUB( test_fixture, (hello) )

int main( int argc, char** argv ) {
   slog( "starting runner" );
   mace::stub::ptr<test_fixture> serv( std::make_shared<test_fixture>() );

   typedef mace::rpc::json::pipe::connection<> connection;
   auto c = connection::ptr( new connection( std::cin, std::cout ) );

   slog( "adding runner interface" );
   mace::stub::visit( serv, connection::add_interface_visitor<test_fixture>( *c, serv ) );

   
   mace::cmt::exec();

   return -9;
}
