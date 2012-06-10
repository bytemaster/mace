#ifndef _MACE_RPC_UDP_SERVER_HPP_
#define _MACE_RPC_UDP_SERVER_HPP_
#include <mace/rpc/server.hpp>
#include <mace/rpc/udp/datagram.hpp>
#include <mace/cmt/asio/udp/socket.hpp>
#include <mace/cmt/asio.hpp>

namespace mace { namespace rpc { namespace udp {

  typedef boost::asio::ip::udp::endpoint endpoint;
	typedef boost::asio::ip::udp::socket socket_t;
  typedef std::shared_ptr<socket_t>  socket_ptr;

  /**
   *  Listens on a given UDP port for messages and creates a 
   *  new connection for each unique endpoint it receives messages
   *  from.  The reply message will come from the new connection.
   *
   *  UDP RPC connections update their 'reply endpoint' any time they get
   *  a new message.  
   *
   *  If the client keeps sending messages to the server endpoint, then the server
   *  will forward them on to the proper connection.
   */
  template<typename InterfaceType, typename ConnectionType>
  class server : public mace::rpc::server<InterfaceType,ConnectionType> {
    public:
      typedef ConnectionType                connection_type;
      typedef typename ConnectionType::ptr  connection_ptr_type;

      template<typename SessionType>
      server( const boost::function<std::shared_ptr<SessionType>()>& sg, const endpoint& listen_ep );

      template<typename SessionType>
      server( const std::shared_ptr<SessionType>& shared_session, const endpoint& listen_ep );

      ~server();

    private:
      void on_connection( const typename ConnectionType::ptr& c );
      void on_disconnect( const typename ConnectionType::ptr& c );
      void listen( const endpoint& p );

      socket_ptr                                                      sock;
      mace::cmt::future<void>                                         listen_complete;
      std::map<mace::rpc::udp::endpoint,typename ConnectionType::ptr> ep_to_con;
      std::map<typename ConnectionType::ptr,boost::any>               connections;
  };

} } } // mace::rpc::udp

#include <mace/rpc/udp/server.ipp>

#endif
