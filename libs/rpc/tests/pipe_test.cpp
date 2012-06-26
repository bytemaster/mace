#include <boost/test/unit_test.hpp>
#include <mace/rpc/value.hpp>
#include <mace/rpc/value_io.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <mace/rpc/json/io.hpp>
#include <boost/process/all.hpp>


#include <mace/rpc/raw/tcp/connection.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <mace/rpc/json/pipe/connection.hpp>
#include <mace/rpc/json/pipe/client.hpp>
#include <mace/rpc/json/tcp/client.hpp>
#include <iostream>

class test_fixture {
  public:
    std::string hello( std::string w ) { return "Hi, " + w; }
};


MACE_STUB( test_fixture, (hello) )


namespace bp = boost::process;
namespace bpb = boost::process::behavior;

int main( int argc, char** argv ) {

   std::string runner = "runner";
   boost::process::context ctx;
   ctx.streams[boost::process::stdout_id] = boost::process::behavior::pipe();
   ctx.streams[boost::process::stdin_id] = boost::process::behavior::pipe();
   boost::process::child c = boost::process::create_child(runner, std::vector<std::string>(), ctx );

   typedef mace::rpc::json::pipe::connection<> connection;

   slog( "creating pipes" );
   boost::process::pistream in( c.get_handle(boost::process::stdout_id) );
   boost::process::postream out( c.get_handle(boost::process::stdin_id) );
   auto con = connection::ptr( new connection( in, out ) );
   
   slog( "creating client" );
   mace::rpc::json::pipe::client<test_fixture> cl(con);

   slog( "sending hello!" );
   std::cout<< cl->hello( "World" ).wait()<<std::endl;
   return 0;
}
