#ifndef _MACE_RPC_RAW_UDP_SERVER_HPP_
#define _MACE_RPC_RAW_UDP_SERVER_HPP_
#include <mace/rpc/udp/server.hpp>
#include <mace/rpc/raw/udp/connection.hpp>

namespace mace { namespace rpc { namespace raw { namespace udp {

#ifndef BOOST_NO_TEMPLATE_ALIASES
  template<typename Interface, typename IODelegate>
  using server = mace::rpc::udp::server<Interface,mace::rpc::raw::udp::connection<IODelegate> >;
#else // BOOST_NO_TEMPLATE_ALIASES 

  /**
   *  A UDP server that accepts connections using the rpc::raw::message
   *  transport and using IODelegate to pass parameters/results.  Defaults to raw_io
   *  for parameters and results.
   *
   *  This is really just a work around for lack of template alias's in VC++
   *
   *  @note this is a workaround for lack of template alias support on anything but g++ 4.7
   */
  template<typename Interface, typename IODelegate=mace::rpc::raw_io>
  class server : public mace::rpc::udp::server<Interface,mace::rpc::raw::udp::connection<IODelegate> > {
    public:
      typedef mace::rpc::raw::udp::connection<IODelegate> connection_type;

      template<typename SessionType>
      server( const boost::function<std::shared_ptr<SessionType>()>& sg, const mace::rpc::udp::endpoint& port )
      :mace::rpc::udp::server<Interface,connection_type>( sg, port ){}

      template<typename SessionType>
      server( const std::shared_ptr<SessionType>& shared_session, const mace::rpc::udp::endpoint& port )
      :mace::rpc::udp::server<Interface,connection_type>( shared_session, port ){}
  };
#endif

} } } } // mace::rpc::raw::udp

#endif // _MACE_RPC_RAW_UDP_SERVER_HPP_
