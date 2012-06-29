#ifndef _MACE_SSH_CLIENT_DETAIL_HPP_
#define _MACE_SSH_CLIENT_DETAIL_HPP_
#include <mace/ssh/client.hpp>
#include <mace/cmt/asio/tcp/socket.hpp>
#include <boost/bind.hpp>
#include <mace/ssh/error.hpp>
#include <libssh2.h>

namespace mace { namespace ssh {
  namespace detail { 


    /**
     *  Internal Implementation of client.
     */
    struct client_d {
        client& self;
        client_d( client& c )
        :self(c), m_session(0),m_knownhosts(0)
        { }

        std::unique_ptr<boost::asio::ip::tcp::socket> m_sock;

        LIBSSH2_SESSION*            m_session;
        LIBSSH2_KNOWNHOSTS*         m_knownhosts;

        mace::cmt::promise<boost::system::error_code>::ptr read_prom;
        mace::cmt::promise<boost::system::error_code>::ptr write_prom;

        /**
         *  Call libssh function, check for EAGAIN, if EAGAIN
         *  then we must ask ASIO to notify us when the socket
         *  is ready and then yield until then.
         */
        template<typename Functor, typename CompareResult>
        CompareResult wait_on_read( Functor&& f, CompareResult test, const char* message = "read" ) {
          using namespace std::placeholders;
          auto rtn = f();
          while( rtn == test ) {
            // are we already waiting for data?
            if( !read_prom ) {
               read_prom.reset( new mace::cmt::promise<boost::system::error_code>() );
               auto s = self.shared_from_this();
               m_sock->async_read_some( boost::asio::null_buffers(),
                                        [s,this]( const boost::system::error_code& e, size_t  ) {
                                          s->my->read_prom->set_value(e);
                                        } );
            }
            // save the promise(so it doesn't go out of scope
            auto prom = read_prom;

            // wait for the result, if an error occured throw
            if( auto e = prom->wait() ) {
               BOOST_THROW_EXCEPTION( boost::system::system_error( e ) );    
            }

            // anyone else who comes along must spawn a new wait promise
            read_prom.reset();
            rtn = f();
          }
          if( rtn < 0 ) {
            char* msg;
                  libssh2_session_last_error( m_session, &msg, 0, 0 );
                  elog( "%1%",msg);
            MACE_SSH_THROW( "%1% failed with %2%: %3%", %message %rtn %msg );
          }
          return rtn;
        }
        /**
         *  Call libssh function, check for EAGAIN, if EAGAIN
         *  then we must ask ASIO to notify us when the socket
         *  is ready and then yield until then.
         */
        template<typename Functor>
        int wait_on_write( Functor&& f, const char* message = "write" ) {
          using namespace std::placeholders;
          int rtn = f();
          while( rtn == LIBSSH2_ERROR_EAGAIN ) {
            // are we already waiting for write?
            if( !write_prom ) {
               write_prom.reset( new mace::cmt::promise<boost::system::error_code>() );
               auto s = self.shared_from_this();
               m_sock->async_write_some( boost::asio::null_buffers(),
                                        [s,this]( const boost::system::error_code& e, size_t  ) {
                                          s->my->write_prom->set_value(e);
                                        } );
            }
            // save the promise(so it doesn't go out of scope
            auto prom = write_prom;

            // wait for the result, if an error occured throw
            if( auto e = prom->wait() ) {
               BOOST_THROW_EXCEPTION( boost::system::system_error( e ) );    
            }

            // anyone else who comes along must spawn a new wait promise
            write_prom.reset(0);
            rtn = f();
          }
          if( rtn < 0 ) {
            MACE_SSH_THROW( "%1% failed with %2%", %message %f );
          }
          return rtn;
        }



        mace::ssh::key host_key;
        std::string    uname;
        std::string    upass;
        std::string    pubkey;
        std::string    privkey;
        std::string    hostname;
        uint16_t       port;
        bool           session_connected;

        bool validate_remote_host();
        bool try_auth();
        bool try_pass();
        bool try_pub_key();
  };
} } } // mace::ssh::detail
#endif // _MACE_SSH_CLIENT_DETAIL_HPP_
