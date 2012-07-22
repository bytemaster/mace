#define BOOST_TEST_MODULE MACE_CMT_ASIO
#include <boost/test/unit_test.hpp>
#include <mace/cmt/asio.hpp>
#include <mace/cmt/thread.hpp>

typedef boost::asio::ip::tcp::socket socket_t;
using namespace boost::asio::ip;

void listen_test( uint16_t p ) {
  mace::cmt::asio::default_io_service();
  auto acc = std::make_shared<boost::asio::ip::tcp::acceptor>( 
            std::ref(mace::cmt::asio::default_io_service()),
            boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(),9808) );
  boost::system::error_code ec;
  auto  iosp = std::shared_ptr<socket_t>( new socket_t( mace::cmt::asio::default_io_service()  ) );
  slog( "waiting for connection" );
  mace::cmt::asio::tcp::accept( *acc, *iosp);
  slog( "sleeping after connection" );
  mace::cmt::usleep(1000000);
  slog( "writing..." );
  mace::cmt::asio::tcp::iostream o(*iosp);
  o<<"Hello World";
  //std::string hello("world");
  //mace::cmt::asio::write( *iosp, boost::asio::const_buffers_1(hello.c_str(), hello.size())  );
  //iosp->close();
  o.close();
}

BOOST_AUTO_TEST_CASE( async_tcp_acceptor ) {
  mace::cmt::async( []() {listen_test( 9808 );} );
  auto con = mace::cmt::async( []() -> std::shared_ptr<socket_t> {
     slog( "attempting to connect" );
     auto iosp = std::shared_ptr<socket_t>( new socket_t( mace::cmt::asio::default_io_service()  ) );
     mace::cmt::asio::tcp::connect( *iosp, tcp::endpoint( tcp::v4(), 9808 ) );
     return iosp;
  }, "async connect" );
  slog( "waiting for connection to complete" );
  auto c = con.wait();

  auto r = mace::cmt::async( [](){ slog( "print before world" ); } );
  
  mace::cmt::asio::tcp::iostream i(*c);
  std::string line;
  std::getline( i, line );
  slog( "%1%", line );
  r.wait();
}

BOOST_AUTO_TEST_CASE( tcp_acceptor ) {
/*
  mace::cmt::asio::default_io_service();
  auto acc = std::make_shared<boost::asio::ip::tcp::acceptor>( 
            std::ref(mace::cmt::asio::default_io_service()),
            boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(),9808) );
            */
}
