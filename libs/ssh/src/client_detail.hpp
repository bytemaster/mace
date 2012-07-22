#ifndef _MACE_SSH_CLIENT_DETAIL_HPP_
#define _MACE_SSH_CLIENT_DETAIL_HPP_
#include <mace/cmt/future.hpp>
#include <mace/cmt/thread.hpp>
#include <mace/ssh/client.hpp>
#include <mace/cmt/asio/tcp/socket.hpp>
#include <boost/bind.hpp>
#include <mace/ssh/error.hpp>
#include <libssh2.h>
#include <libssh2_sftp.h>
#include <mace/cmt/mutex.hpp>

namespace mace { namespace ssh {
  namespace detail { 
    /**
     *  Internal Implementation of client.
     */
    struct client_d {
        client& self;
        client_d( client& c )
        :self(c), m_session(0),m_knownhosts(0),m_sftp(0),m_shutdown(0)
        { 
          m_thread = &mace::cmt::thread::current(); 
        }

        std::unique_ptr<boost::asio::ip::tcp::socket> m_sock;

        LIBSSH2_SESSION*            m_session;
        LIBSSH2_KNOWNHOSTS*         m_knownhosts;
        LIBSSH2_SFTP*               m_sftp;

        typedef mace::cmt::promise<boost::system::error_code>::ptr rw_prom;
        rw_prom read_prom;
        rw_prom write_prom;
        mace::cmt::mutex scp_send_mutex;
        mace::cmt::thread*          m_thread;
        bool                        m_shutdown;
        

        /**
         *  @pre only one thread can call this method at a time.
         *  @pre libssh2_session_block_directions is set to either INBOUND or OUTBOUND or both. 
         *  @post socket has data available for either INBOUND or OUTBOUND according to the
         *        specified block direction.
         *  @throw boost::system::system_error if an error occurs on the socket while waiting.
         */
        void wait_on_socket() {
          BOOST_ASSERT( m_thread == &mace::cmt::thread::current() ); 

    //      mace::cmt::thread::current().debug("wait_on_socket");
          auto dir = libssh2_session_block_directions(m_session);
          if( !dir ) return;
          BOOST_ASSERT( dir & (LIBSSH2_SESSION_BLOCK_INBOUND | LIBSSH2_SESSION_BLOCK_OUTBOUND ) );

          rw_prom rprom, wprom;
          if( dir & LIBSSH2_SESSION_BLOCK_INBOUND ) {
            rprom = read_prom;
            if(!rprom.get()) {
          //     elog( "   this %2%            NEW READ PROM      %1%           ", read_prom.get(), this );
               read_prom.reset( new mace::cmt::promise<boost::system::error_code>("read_prom") );
           //    wlog( " new read prom %1%   this %2%", read_prom.get(), this );
               rprom = read_prom;
               m_sock->async_read_some( boost::asio::null_buffers(),
                                        [=]( const boost::system::error_code& e, size_t  ) {
                                          this->read_prom->set_value(e);
                                          this->read_prom.reset(0);
                                        } );
            } else {
      //        elog( "already waiting on read %1%", read_prom.get() );
            }
          }
          
          if( dir & LIBSSH2_SESSION_BLOCK_OUTBOUND ) {
            wprom = write_prom;
            if( !write_prom ) {
                write_prom.reset( new mace::cmt::promise<boost::system::error_code>("write_prom") );
                wprom = write_prom;
                m_sock->async_write_some( boost::asio::null_buffers(),
                                         [=]( const boost::system::error_code& e, size_t  ) {
                                            this->write_prom->set_value(e);
                                            this->write_prom.reset(0);
                                         } );
            } else {
        //      elog( "already waiting on write" );
            }
          }


          boost::system::error_code ec;
          if( rprom.get() && wprom.get() ) {
           // elog( "************* Attempt to wait in either direction currently waits for both directions ****** " );
            //wlog( "rprom %1%   wprom %2%", rprom.get(), write_prom.get() );
        //     wlog( "wait on read %1% or write %2% ", rprom.get(), wprom.get() );
            typedef mace::cmt::future<boost::system::error_code> fprom;
            fprom fw(wprom);
            fprom fr(rprom);
            int r = mace::cmt::wait_any( fw, fr );
            switch( r ) {
              case 0:
                break;
              case 1:
                break;
            }
          } else if( rprom ) {
           //   wlog( "                                                             wait on read %1%", rprom.get() );
             // mace::cmt::thread::current().debug("wait on read ");
              if( rprom->wait() ) { 
                BOOST_THROW_EXCEPTION( boost::system::system_error(rprom->wait() ) ); 
              }
            //  wlog( "                                                             done wait on read %1%", rprom.get() );
      //        mace::cmt::thread::current().debug("read ready");
          } else if( wprom ) {
             // wlog( "wait on write %1%", wprom.get() );
              if( wprom->wait() ) { BOOST_THROW_EXCEPTION( boost::system::system_error(wprom->wait() ) ); }
             // wlog( "WRITE READY %1%", wprom.get() );
          }
       //   mace::cmt::thread::current().debug("end wait_on_socket");
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
            client_d* self = (client_d*)*abstract;

    //        printf("Performing keyboard-interactive authentication.\n");

     //       printf("Authentication name: '");
      //      fwrite(name, 1, name_len, stdout);
      //      printf("'\n");

       //     printf("Authentication instruction: '");
       //     fwrite(instruction, 1, instruction_len, stdout);
       //     printf("'\n");

       //     printf("Number of prompts: %d\n\n", num_prompts);

            for (i = 0; i < num_prompts; i++) {
        //        printf("Prompt %d from server: '", i);
                fwrite(prompts[i].text, 1, prompts[i].length, stdout);
          //      printf("'\n");

         //       printf("Please type response: ");

                if( self->upass.size() == 0 ) {
                    fgets(buf, sizeof(buf), stdin);
                    n = strlen(buf);
                    while (n > 0 && strchr("\r\n", buf[n - 1]))
                      n--;
                    buf[n] = 0;

                    responses[i].text = _strdup(buf);
                    responses[i].length = n;
                } else {
                    responses[i].text = _strdup(self->upass.c_str());
                    responses[i].length = self->upass.size();
                }

             //   printf("Response %d from user is '", i);
             //   fwrite(responses[i].text, 1, responses[i].length, stdout);
             //   printf("'\n\n");
            }

         //   printf("Done. Sending keyboard-interactive responses to server now.\n");
        }


        LIBSSH2_CHANNEL*   open_channel( const std::string& pty_type ) {
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

            if( pty_type.size() ) {
                int ec = libssh2_channel_request_pty(chan,pty_type.c_str());
                while( ec == LIBSSH2_ERROR_EAGAIN ) {
                   wait_on_socket();
                   ec = libssh2_channel_request_pty(chan,pty_type.c_str());
                }
                if( 0 != ec ) {
                   char* msg;
                   ec = libssh2_session_last_error( m_session, &msg, 0, 0 );
                   MACE_SSH_THROW( "libssh2_channel_req_pty failed: %1% - %2%", %ec %msg  );
                }
            }
            return chan;
        }

        void connect();


        //mace::ssh::key                 host_key;
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

        void init_sftp() {
          if( !m_sftp ) {
             m_sftp = libssh2_sftp_init(m_session);
             while( !m_sftp ) {
                char* msg = 0;
                int   ec = libssh2_session_last_error(m_session,&msg,NULL,0);
                if( ec == LIBSSH2_ERROR_EAGAIN ) {
                  wait_on_socket();
                  m_sftp = libssh2_sftp_init(m_session);
                } else {
                  MACE_SSH_THROW( "init sftp error %1%: %2%", %ec %msg );
                }
             }
          }
        }
  };
} } } // mace::ssh::detail
#endif // _MACE_SSH_CLIENT_DETAIL_HPP_
