#ifndef _MACE_SSH_CLIENT_DETAIL_HPP_
#define _MACE_SSH_CLIENT_DETAIL_HPP_
#include <mace/cmt/future.hpp>
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
         *  @pre libssh2_session_block_directions is set to either INBOUND or OUTBOUND or both. 
         *  @post socket has data available for either INBOUND or OUTBOUND according to the
         *        specified block direction.
         *  @throw boost::system::system_error if an error occurs on the socket while waiting.
         */
        void wait_on_socket() {
          auto dir = libssh2_session_block_directions(m_session);
          BOOST_ASSERT( dir & (LIBSSH2_SESSION_BLOCK_INBOUND | LIBSSH2_SESSION_BLOCK_OUTBOUND ) );

          mace::cmt::future<boost::system::error_code> rprom, wprom;
          if( dir & LIBSSH2_SESSION_BLOCK_INBOUND ) {
            if(!read_prom) {
               read_prom.reset( new mace::cmt::promise<boost::system::error_code>() );
               auto s = self.shared_from_this();
               m_sock->async_read_some( boost::asio::null_buffers(),
                                        [=]( const boost::system::error_code& e, size_t  ) {
                                          read_prom->set_value(e);
                                        } );
            }
            rprom = mace::cmt::future<boost::system::error_code>(read_prom);
          }
          
          if( dir & LIBSSH2_SESSION_BLOCK_OUTBOUND ) {
             if( !write_prom ) {
                write_prom.reset( new mace::cmt::promise<boost::system::error_code>() );
                auto s = self.shared_from_this();
                m_sock->async_write_some( boost::asio::null_buffers(),
                                         [=]( const boost::system::error_code& e, size_t  ) {
                                            write_prom->set_value(e);
                                         } );
            }
            wprom = mace::cmt::future<boost::system::error_code>(write_prom);
          }
          boost::system::error_code ec;
          if( rprom.valid() && wprom.valid() ) {
            /// TODO: implement wait on any in mace::cmt
            wlog( "Attempt to wait in either direction currently waits for both directions" );
            slog( "... wait for write..." );
            if( wprom.wait() ) { BOOST_THROW_EXCEPTION( boost::system::system_error(wprom.wait() ) ); }
            slog( "... wait for read..." );
            if( rprom.wait() ) { BOOST_THROW_EXCEPTION( boost::system::system_error(rprom.wait() ) ); }
            /*
              int p = mace::cmt::wait_any( rprom, wprom );
              switch( p ) {
                case 0:
                  if( rprom.wait() ) { BOOST_THROW_EXCEPTION( boost::system::system_error(wprom.wait() ) ); }
                case 1:
                  if( wprom.wait() ) { BOOST_THROW_EXCEPTION( boost::system::system_error(wprom.wait() ) ); }
              }
              BOOST_ASSERT( p == 0 || p == 1 );
              return;
            */
          } else if( rprom.valid() ) {
              if( rprom.wait() ) { BOOST_THROW_EXCEPTION( boost::system::system_error(rprom.wait() ) ); }
              read_prom.reset(0);
          } else if( wprom.valid() ) {
              if( wprom.wait() ) { BOOST_THROW_EXCEPTION( boost::system::system_error(wprom.wait() ) ); }
              write_prom.reset(0);
          }
        }

        static void kbd_callback(const char *name, int name_len, 
                     const char *instruction, int instruction_len, int num_prompts,
                     const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
                     LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,
                     void **abstract)
        {
            int i;
            size_t n;
            char buf[1024];
            (void)abstract;

            printf("Performing keyboard-interactive authentication.\n");

            printf("Authentication name: '");
            fwrite(name, 1, name_len, stdout);
            printf("'\n");

            printf("Authentication instruction: '");
            fwrite(instruction, 1, instruction_len, stdout);
            printf("'\n");

            printf("Number of prompts: %d\n\n", num_prompts);

            for (i = 0; i < num_prompts; i++) {
                printf("Prompt %d from server: '", i);
                fwrite(prompts[i].text, 1, prompts[i].length, stdout);
                printf("'\n");

                printf("Please type response: ");
                fgets(buf, sizeof(buf), stdin);
                n = strlen(buf);
                while (n > 0 && strchr("\r\n", buf[n - 1]))
                  n--;
                buf[n] = 0;

                responses[i].text = strdup(buf);
                responses[i].length = n;

                printf("Response %d from user is '", i);
                fwrite(responses[i].text, 1, responses[i].length, stdout);
                printf("'\n\n");
            }

            printf("Done. Sending keyboard-interactive responses to server now.\n");
        }


        LIBSSH2_CHANNEL*   open_channel() {
            LIBSSH2_CHANNEL*                      chan = 0;
            chan = libssh2_channel_open_session(m_session);
            if( !chan ) {
               char* msg;
               int ec = libssh2_session_last_error( m_session, &msg, 0, 0 );
               while( !chan && ec == LIBSSH2_ERROR_EAGAIN ) {
                  wait_on_socket();
                  chan = libssh2_channel_open_session(m_session);
                  ec   = libssh2_session_last_error( m_session, &msg, 0, 0 );
               }
               if( !chan ) {
                  MACE_SSH_THROW( "libssh2_channel_open_session failed: %1% - %2%", %ec %msg  );
               }
            }
            return chan;
        }



        mace::ssh::key                 host_key;
        boost::asio::ip::tcp::endpoint endpt;
        std::string                    uname;
        std::string                    upass;
        std::string                    pubkey;
        std::string                    privkey;
        std::string                    passphrase;
        std::string                    hostname;
        uint16_t                       port;
        bool                           session_connected;

        bool validate_remote_host();
        void authenticate();
        bool try_pass();
        bool try_pub_key();
        bool try_keyboard();
  };
} } } // mace::ssh::detail
#endif // _MACE_SSH_CLIENT_DETAIL_HPP_
