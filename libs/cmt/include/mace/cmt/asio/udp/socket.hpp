/** 
 * @file mace/cmt/asio/udp/socket.hpp
 * @brief defines cooperative / synchronous receive_from() and send_to() for boost::asio::ip::udp::socket
 *
 */
#ifndef _MACE_CMT_ASIO_UDP_SOCKET_HPP_
#define _MACE_CMT_ASIO_UDP_SOCKET_HPP_
#include <boost/asio.hpp>

namespace mace { namespace cmt { namespace asio { namespace udp {

    size_t receive_from( boost::asio::ip::udp::socket& s, char* data, size_t data_len, boost::asio::ip::udp::endpoint& ep );
    size_t send_to( boost::asio::ip::udp::socket& s, const char* data, size_t data_len, const boost::asio::ip::udp::endpoint& ep );

} } } } // namespace mace::cmt::asio::udp

#endif // _MACE_CMT_ASIO_UDP_SOCKET_HPP_

