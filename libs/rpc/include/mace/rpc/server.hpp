#ifndef _MACE_RPC_SERVER_HPP_
#define _MACE_RPC_SERVER_HPP_
#include <mace/cmt/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <mace/stub/ptr.hpp>

namespace mace { namespace rpc { 
 
  /**
   *  @brief listens for incoming connections and create
   *  a new session for servicing those connections.
   *
   *  This is the base class that implements the session creation and
   *  method registration code.  Derived classes will provide UDP,TCP,HTTP,
   *  or other server types.  The server is templated upon the type of
   *  connection that it will be accepting.
   */
  template<typename InterfaceType, typename ConnectionType>
  class server {
    public:
      typedef std::shared_ptr<server> ptr;
      typedef ConnectionType            connection_type;
      struct session_creator {
          virtual ~session_creator(){}
          virtual boost::any init_connection( const typename ConnectionType::ptr& ) = 0;
      };

      boost::scoped_ptr<session_creator> sc;
      template<typename SessionType>
      server( const boost::function<std::shared_ptr<SessionType>()>& sg )
      :sc( new session_creator_impl<SessionType>(sg) ){}

      template<typename SessionType>
      server( const std::shared_ptr<SessionType>& shared_session )
      :sc( new shared_session_creator<SessionType>(shared_session) ){}

    protected:
      template<typename SessionType>
      struct session_creator_impl : public session_creator {
          session_creator_impl( const boost::function<std::shared_ptr<SessionType>()>& sg )
          :session_generator(sg){ }

          virtual boost::any init_connection( const typename ConnectionType::ptr& con ) {
            mace::stub::ptr<InterfaceType> session( session_generator() );
            mace::stub::visit( session, typename ConnectionType::template add_interface_visitor<InterfaceType>( *con, session ) );
            return session;
          }
          boost::function<std::shared_ptr<SessionType>()> session_generator;
      };

      template<typename SessionType>
      struct shared_session_creator : public session_creator {
          shared_session_creator( const std::shared_ptr<SessionType>& ss ):shared_session(ss){}

          virtual boost::any init_connection( const typename ConnectionType::ptr& con ) {
            mace::stub::ptr<InterfaceType> session;
            session = shared_session;
            mace::stub::visit( session, typename ConnectionType::template add_interface_visitor<InterfaceType>( *con, session ) );
            return session;
          }
          std::shared_ptr<SessionType> shared_session;
      };
  };

} }  // mace::rpc

#endif // _MACE_RPC_JSON_TCP_SERVER_HPP_
