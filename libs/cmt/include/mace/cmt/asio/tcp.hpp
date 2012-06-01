#ifndef _BOOST_CMT_ASIO_TCP_HPP_
#define _BOOST_CMT_ASIO_TCP_HPP_
#include <boost/cmt/asio/basic_socket_iostream.hpp>
namespace boost { namespace cmt { namespace asio { 
namespace tcp {
    typedef boost::cmt::asio::basic_socket_iostream< boost::asio::ip::tcp> iostream;
}
} } }
#endif // _BOOST_CMT_ASIO_TCP_HPP_
