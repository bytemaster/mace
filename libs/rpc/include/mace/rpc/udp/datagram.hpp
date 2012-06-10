#ifndef _MACE_RPC_UDP_DATAGRAM_HPP_
#define _MACE_RPC_UDP_DATAGRAM_HPP_
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <utility>
#include <vector>

namespace mace { namespace rpc { namespace udp {

  typedef boost::asio::ip::udp::endpoint endpoint;

  /**
   *  @brief used for currying UDP packets around.
   *
   *  This class is noncopyable in order to force move semantics,
   *  not because we 'couldn't' copy it.
   */
  struct datagram : boost::noncopyable {
      datagram( size_t s, const endpoint& ep = endpoint() )
      :data(s){}

      datagram( datagram&& dg )
      :data(std::move(dg.data)),ep(std::move(dg.ep)){}

      datagram( std::vector<char>&& _data, endpoint&& _ep )
      :data(std::move(_data)),ep(std::move(_ep)){}

      datagram& operator==( datagram&& dg ) {
        std::swap( data,dg.data);
        std::swap( ep, dg.ep );
        return *this;
      }

      std::vector<char> data;
      endpoint          ep;
  };

} } }

#endif
