#define BOOST_TEST_MODULE MACE_CMT_ASIO
#include <boost/test/unit_test.hpp>
#include <mace/cmt/asio.hpp>
#include <mace/cmt/asio/tcp/socket.hpp>
#include <mace/cmt/thread.hpp>

void listen_test( uint16_t p ) {
  mace::cmt::asio::default_io_service();
  auto acc = std::make_shared<boost::asio::ip::tcp::acceptor>( 
            std::ref(mace::cmt::asio::default_io_service()),
            boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(),9808) );
  boost::system::error_code ec;
  typedef mace::cmt::asio::tcp::socket socket_t;
  socket_t::ptr iosp(new socket_t());
  ec = mace::cmt::asio::tcp::accept( *acc, *iosp);
}

BOOST_AUTO_TEST_CASE( async_tcp_acceptor ) {
  mace::cmt::async( std::bind( listen_test, 9808 ) ).wait();
}

BOOST_AUTO_TEST_CASE( tcp_acceptor ) {
  mace::cmt::asio::default_io_service();
  auto acc = std::make_shared<boost::asio::ip::tcp::acceptor>( 
            std::ref(mace::cmt::asio::default_io_service()),
            boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(),9808) );
}
