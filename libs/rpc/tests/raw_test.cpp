#define BOOST_TEST_MODULE MACE_RPC_VALUE
#include <boost/test/unit_test.hpp>
#include <mace/rpc/value.hpp>
#include <mace/rpc/value_io.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <mace/rpc/json/io.hpp>


#include <mace/rpc/raw/tcp/connection.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <mace/rpc/raw/raw_io.hpp>
#include <mace/rpc/raw/tcp/server.hpp>
#include <mace/rpc/json/tcp/server.hpp>
#include <mace/rpc/client.hpp>
#include <mace/rpc/json/tcp/client.hpp>
#include <iostream>

class test_fixture {
  public:
    int test_cb( const boost::function<std::string(int)>& f ) { 
      cb_result = f(6); 
      return 5;
    }
    std::string cb_result;
};

struct client_test {
  std::string test_func( int p ) {
    test_func_param = p;
    return "hello world";
  }
  int test_func_param;
};


MACE_STUB( test_fixture, (test_cb) )


BOOST_AUTO_TEST_CASE( jsonrpc_callback_test ) {
  
  auto s = std::make_shared<test_fixture>();
  mace::rpc::json::tcp::server<test_fixture>  serv( s, 9999 );
  mace::rpc::json::tcp::client<test_fixture> c;
  c.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string( "127.0.0.1" ), 
                9999  ) );
 
  client_test ct;
  int result = c->test_cb( boost::function<std::string(int)>(boost::bind( &client_test::test_func, &ct, _1 )) );

  BOOST_REQUIRE( ct.test_func_param == 6 );
  BOOST_REQUIRE( s->cb_result == "hello world" );
  BOOST_REQUIRE( result      == 5 );
}


BOOST_AUTO_TEST_CASE( rawrpc_callback_test ) {
  
  auto s = std::make_shared<test_fixture>();
  mace::rpc::raw::tcp::server<test_fixture>  serv( s, 9999 );
  mace::rpc::client<test_fixture> c;
  c.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string( "127.0.0.1" ), 
                9999  ) );
 
  client_test ct;
  int result = c->test_cb( std::function<std::string(int)>(boost::bind( &client_test::test_func, &ct, _1 )) );

  BOOST_REQUIRE( ct.test_func_param == 6 );
  BOOST_REQUIRE( s->cb_result == "hello world" );
  BOOST_REQUIRE( result      == 5 );
}

