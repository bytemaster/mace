#include <mace/cmt/asio/tcp/socket.hpp>
#include <mace/ssh/client.hpp>
#include <mace/ssh/error.hpp>
#include <mace/cmt/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "client_detail.hpp"
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace mace { namespace ssh {
  namespace detail { 
  /*
        bool client_d::validate_remote_host() {
          size_t len;
          int type;
          const char * fingerprint = libssh2_session_hostkey(m_session, &len, &type);
          d_hostKey.key=std::vector<char>(fingerprint,fingerprint+len);
          auto hkh = libssh2_hostkey_hash(d_session,LIBSSH2_HOSTKEY_HASH_MD5);
          d_hostKey.hash=std::vector<char>(hkh,hkh+16);
          switch (type){
              case LIBSSH2_HOSTKEY_TYPE_RSA:
                  host_key.type = mace::cmt::ssh::client::key::rsa;
                  break;
              case LIBSSH2_HOSTKEY_TYPE_DSS:
                  host_key.type = mace::cmt::ssh::client::key::dss; 
                  break;
              default:
                  host_key.type = mace::cmt::ssh::client::key::unknown;
          }
          if(fingerprint) {
              struct libssh2_knownhost *host;
              int check = libssh2_knownhost_check(m_knownhosts, hostname.c_str(),
                                                  (char *)fingerprint, len,
                                                  LIBSSH2_KNOWNHOST_TYPE_PLAIN|
                                                  LIBSSH2_KNOWNHOST_KEYENC_RAW,
                                                  &host);
          
              switch(check){
                  case LIBSSH2_KNOWNHOST_CHECK_MATCH:
                      return true;
                  case LIBSSH2_KNOWNHOST_CHECK_FAILURE:
                      MACE_CMT_THROW( "Host Key Invalid" );
                      break;
                  case LIBSSH2_KNOWNHOST_CHECK_MISMATCH:
                      MACE_CMT_THROW( "Host Key Mismatch" );
                      break;
                  case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND:
                      MACE_CMT_THROW( "Host Key Unknown Error" );
                      break;
              }
          } else {
              MACE_CMT_THROW( "Host Key Invalid" );
          }
        }
        */

        /** 
         *  Request the list of authorization methods from the server and
         *  attempt to authenticate.
         *
         *  @pre m_session is valid
         *  @pre uname has been set
         *  @post authenticated user.
         *
         *  @throw if not authenticated
         */
        void client_d::authenticate() {
            BOOST_ASSERT( uname.size() );
            BOOST_ASSERT( m_session );

            char * alist = libssh2_userauth_list(m_session, uname.c_str(),uname.size());
            char * msg   = 0;
            int    ec    = 0;

            if(alist==NULL){
                if(libssh2_userauth_authenticated(m_session)){
                    return; // CONNECTED!
                } 
                ec = libssh2_session_last_error(m_session,&msg,NULL,0);

                while( !alist && (ec == LIBSSH2_ERROR_EAGAIN) ) {
                    wait_on_socket();
                    alist = libssh2_userauth_list(m_session, uname.c_str(), uname.size());
                    ec = libssh2_session_last_error(m_session,&msg,NULL,0);
                }
                if( !alist ) {
                    MACE_SSH_THROW( "Error getting authorization list: %1% - %2%", %ec %msg );
                }
            }

            std::vector<std::string> split_alist;
            bool pubkey = false;
            bool pass   = false;
            bool keybd   = false;
            boost::split( split_alist, alist, boost::is_any_of(",") );
            std::for_each( split_alist.begin(), split_alist.end(), [&](const std::string& s){
              if( s == "publickey" ) {
                pubkey = true;
              }
              else if( s == "password" ) {
                pass = true;
              }
              else if( s == "keyboard-interactive" ) {
                keybd = true;
              }
              else {
                slog( "Unknown/unsupported authentication type '%1%'", s );
              }
            });

            if( pubkey ) {
              if( try_pub_key() ) 
                return;
            }
            if( pass ) {
              if( try_pass() )
                return;
            }
            if( keybd ) {
              if( try_keyboard() )
                return;
            }
            MACE_SSH_THROW( "Unable to authenticate" );
        }

        bool client_d::try_pass() {
            BOOST_ASSERT( m_session );
            BOOST_ASSERT( uname.size() );

            int ec = libssh2_userauth_password(m_session, uname.c_str(), upass.c_str() );
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
              wait_on_socket();
              ec = libssh2_userauth_password(m_session, uname.c_str(), upass.c_str() );
            }

            return !ec;
        }
        bool client_d::try_keyboard() {
            BOOST_ASSERT( m_session );
            BOOST_ASSERT( uname.size() );
            
            int ec = libssh2_userauth_keyboard_interactive(m_session, uname.c_str(), &client_d::kbd_callback);
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
              wait_on_socket();
              ec = libssh2_userauth_keyboard_interactive(m_session, uname.c_str(), &client_d::kbd_callback);
            }
            return !ec;
        }

        bool client_d::try_pub_key() {
            BOOST_ASSERT( m_session );
            BOOST_ASSERT( uname.size() > 0  );

            int ec = libssh2_userauth_publickey_fromfile(m_session,
                                                       uname.c_str(),
                                                       pubkey.c_str(),
                                                       privkey.c_str(),
                                                       passphrase.c_str() );
   
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
              wait_on_socket();
              ec = libssh2_userauth_publickey_fromfile(m_session,
                                                       uname.c_str(),
                                                       pubkey.c_str(),
                                                       privkey.c_str(),
                                                       passphrase.c_str() );
            }
            return !ec;
        }


  } // namespace detail

  client::ptr client::create() {
    client::ptr c = client::ptr(new client());
    return c;
  }

  client::client() {
    my = new detail::client_d(*this);
  }

  client::~client() {
    try { close(); } catch( ... ) {}
    delete my;
  }


  /**
   *  Connect and authenticate or throw.
   *
   *  This method will resolve host and connect to the first valid endpoint.
   *
   *  @pre not already connected (ASSERT)
   *  @pre host resolves to a valid endpoint
   *  @pre valid user name
   *  @pre ssh keys for user have been established for the specified host
   *  @pre host is running a sshd server on port
   *
   *  @post a valid SSH session has been started.
   *  @post m_session != NULL
   *  @post libssh2 library initialized.
   *
   *  @throw if any of the run-time pre conditions are not met.
   */
  void client::connect( const std::string& user, const std::string& host, uint16_t port ) {
    try {
       // not connected
       BOOST_ASSERT( !my->m_session );
       
       my->hostname = host;
       my->uname    = user;
       my->port     = port;
       
       if( libssh2_init(0) < 0  ) {
         MACE_SSH_THROW( "Unable to init libssh2" );
       }
       
       slog( "resolve %1%:%2%", my->hostname, port );
       std::vector<mace::cmt::asio::tcp::endpoint> eps 
         = mace::cmt::asio::tcp::resolve( my->hostname, boost::lexical_cast<std::string>(port));
       slog( "resolved %1% options", eps.size() );
       
       if( eps.size() == 0 ) {
         MACE_SSH_THROW( "Hostname '%1%' didn't resolve to any endpoints", %host );
       }
       
       my->m_sock.reset( new boost::asio::ip::tcp::socket( mace::cmt::asio::default_io_service() ) );
       
       for( uint32_t i = 0; i < eps.size(); ++i ) {
          boost::system::error_code e = mace::cmt::asio::tcp::connect( *my->m_sock, eps[i] );
          if( !e ) {
            slog( "connected to remote endpoint" );
            my->endpt = eps[i];
            break;
          }
       }

       slog( "Creating session" );
       my->m_session = libssh2_session_init(); 
       BOOST_ASSERT( my->m_session );
       
       // use non-blocking calls so that we know when to call my->wait_on_socket
       libssh2_session_set_blocking( my->m_session, 0 );
       
       // perform the session handshake, and keep trying while EAGAIN
       int ec = libssh2_session_handshake( my->m_session, my->m_sock->native() );
       while( ec == LIBSSH2_ERROR_EAGAIN ) {
         my->wait_on_socket();
         ec = libssh2_session_handshake( my->m_session, my->m_sock->native() );
       }

       // if there was an error, throw it.
       if( ec < 0 ) {
         char* msg;
         libssh2_session_last_error( my->m_session, &msg, 0, 0 );
         MACE_SSH_THROW( "Handshake error: %1% - %2%", %ec %msg );
       }
       
       /* At this point we havn't yet authenticated.  The first thing to do
        * is check the hostkey's fingerprint against our known hosts Your app
        * may have it hard coded, may go to a file, may present it to the
        * user, that's your call
        *
        * TODO: validate fingerprint
        */
       const char* fingerprint = libssh2_hostkey_hash(my->m_session, LIBSSH2_HOSTKEY_HASH_SHA1);
        
       // try to authenticate, throw on error.
       my->authenticate();
       
    } catch ( ... ) {
      close();
      throw;
    }
  }

  process::ptr client::exec( const std::string& cmd ) {
    process::ptr cpp(new process(*this,cmd));
    //my->processes.push_back(ccp);
    return cpp;
  }

  /**
   *  Handles uploading file to remote host via scp.  The file will be memory mapped and then
   *  copied. As the copy progresses, the progress callback will be called.  To cancel the upload
   *  the progress callback should return 'false'.  
   *
   *  @param progress - called with progress updates as the file is uploaded.
   *  
   *  @pre local_path exists
   *  @pre remote_path all directories in the remote path exist.
   */
  void  client::scp_send( const std::string& local_path, const std::string& remote_path, 
                     boost::function<bool(size_t,size_t)> progress  ) {
    slog( "scp send %1%", local_path );
    using namespace boost::filesystem;
    if( !exists(local_path) ) {
      MACE_SSH_THROW( "Source file '%1%' does not exist", %local_path );
    }
    if( is_directory( local_path ) ) {
      MACE_SSH_THROW( "Source path '%1%' is a directory, expected a file.", %local_path );
    }

    using namespace boost::interprocess;
    // memory map the file
    file_mapping fmap( local_path.c_str(), read_only );
    size_t       fsize = file_size(local_path);

    mapped_region mr( fmap, boost::interprocess::read_only, 0, fsize );

    LIBSSH2_CHANNEL*                      chan = 0;
    time_t now;
    memset( &now, 0, sizeof(now) );
    // TODO: preserve creation / modification date
    chan = libssh2_scp_send64( my->m_session, remote_path.c_str(), 0700, fsize, now, now );
    while( chan == 0 ) {
      char* msg;
      int ec = libssh2_session_last_error( my->m_session, &msg, 0, 0 );
      if( ec == LIBSSH2_ERROR_EAGAIN ) {
        slog( "create chan wait on socket %1%", local_path );
        my->wait_on_socket();
        slog( "done create chan wait on socket %1%", local_path );
        chan = libssh2_scp_send64( my->m_session, local_path.c_str(), 0700, fsize, now, now );
      } else {
          MACE_SSH_THROW( "scp failed %1% - %2%", %ec %msg );
      }
    }
    try {
      uint64_t   wrote = 0;
      char* pos = reinterpret_cast<char*>(mr.get_address());
      while( progress( wrote, fsize ) && wrote < fsize ) {
          int r = libssh2_channel_write( chan, pos, fsize - wrote );
          slog( "scp send %1% r %2%", local_path, r );
          if( r < 0 ) {
            if( r == LIBSSH2_ERROR_EAGAIN ) {
              slog( "wait on socket %1%", local_path );
              my->wait_on_socket();
              slog( "done wait socket %1%", local_path );
              continue;
            } else {
              char* msg = 0;
              int ec = libssh2_session_last_error( my->m_session, &msg, 0, 0 );
              MACE_SSH_THROW( "scp failed %1% - %2%", %ec %msg );
            }
          }
          wrote += r;
          pos   += r;
      } 
    } catch ( ... ) {
      // clean up chan
      int ec = libssh2_channel_free(chan );  
      while( ec == LIBSSH2_ERROR_EAGAIN ) {
        my->wait_on_socket();
        ec = libssh2_channel_free( chan );  
      }
      throw;
    }
    int ec = libssh2_channel_free( chan );  
    while( ec == LIBSSH2_ERROR_EAGAIN ) {
      my->wait_on_socket();
      ec = libssh2_channel_free( chan );  
    }
  }

  /**
   *  @post all session and socket objects closed and freed.
   */
  void client::close() {
    if( my->m_session ) {
       try {
         int ec = libssh2_session_disconnect(my->m_session, "exit cleanly" );
         while( ec == LIBSSH2_ERROR_EAGAIN ) {
            my->wait_on_socket();
            ec = libssh2_session_disconnect(my->m_session, "exit cleanly" );
         }
         ec = libssh2_session_free(my->m_session);
         while( ec == LIBSSH2_ERROR_EAGAIN ) {
            my->wait_on_socket();
            ec = libssh2_session_free(my->m_session );
         }
         my->m_session = 0;
       } catch ( ... ){}
       try {
         if( my->m_sock ) {
           my->m_sock->close();
           my->m_sock.reset(0);
         }
       } catch ( ... ){}
       try {
        //if( my->read_prom ) my->read_prom->wait();
       } catch ( ... ){}
       try {
        //if( my->write_prom ) my->write_prom->wait();
       } catch ( ... ){}
    }
  }


}} // mace::ssh
