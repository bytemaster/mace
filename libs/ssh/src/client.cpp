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
    static int ssh_init = libssh2_init(0);
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

        void client_d::connect() {
            try {
               if( libssh2_init(0) < 0  ) {
                 MACE_SSH_THROW( "Unable to init libssh2" );
               }
               
               slog( "resolve %1%:%2%", hostname, port );
               std::vector<mace::cmt::asio::tcp::endpoint> eps 
                 = mace::cmt::asio::tcp::resolve( hostname, boost::lexical_cast<std::string>(port));
               slog( "resolved %1% options", eps.size() );
               
               if( eps.size() == 0 ) {
                 MACE_SSH_THROW( "Hostname '%1%' didn't resolve to any endpoints", %hostname );
               }
               
               m_sock.reset( new boost::asio::ip::tcp::socket( mace::cmt::asio::default_io_service() ) );
               
               for( uint32_t i = 0; i < eps.size(); ++i ) {
                  try {
                    mace::cmt::asio::tcp::connect( *m_sock, eps[i] );
                    endpt = eps[i];
                    break;
                  } catch ( ... ) {}
               }

               slog( "Creating session" );

               m_session = libssh2_session_init(); 
               *libssh2_session_abstract(m_session) = this;


               BOOST_ASSERT( m_session );
               
               // use non-blocking calls so that we know when to call wait_on_socket
               libssh2_session_set_blocking( m_session, 0 );
               
               // perform the session handshake, and keep trying while EAGAIN
               int ec = libssh2_session_handshake( m_session, m_sock->native() );
               while( ec == LIBSSH2_ERROR_EAGAIN ) {
                 wait_on_socket();
                 ec = libssh2_session_handshake( m_session, m_sock->native() );
               }

               // if there was an error, throw it.
               if( ec < 0 ) {
                 char* msg;
                 libssh2_session_last_error( m_session, &msg, 0, 0 );
                 MACE_SSH_THROW( "Handshake error: %1% - %2%", %ec %msg );
               }
               
               /* At this point we havn't yet authenticated.  The first thing to do
                * is check the hostkey's fingerprint against our known hosts Your app
                * may have it hard coded, may go to a file, may present it to the
                * user, that's your call
                *
                * TODO: validate fingerprint
                */
               const char* fingerprint = libssh2_hostkey_hash(m_session, LIBSSH2_HOSTKEY_HASH_SHA1);
                
               // try to authenticate, throw on error.
               authenticate();

          //     libssh2_trace(m_session, LIBSSH2_TRACE_TRANS);
               
            } catch ( ... ) {
              self.close();
              throw;
            }
        }


  } // namespace detail

  file_attrib::file_attrib()
  :size(0),uid(0),gid(0),permissions(0),atime(0),mtime(0)
  { }
  bool file_attrib::is_directory() {
    return  LIBSSH2_SFTP_S_ISDIR(permissions);
  }
  bool file_attrib::is_file() {
    return LIBSSH2_SFTP_S_ISREG(permissions);
  }
  bool file_attrib::exists() {
    return 0 != permissions;
  }

  client::ptr client::create() {
    client::ptr c = client::ptr(new client());
    return c;
  }

  client::client() {
    my = new detail::client_d(*this);
  }

  client::~client() {
    try { close(); } catch( ... ) { elog(""); }
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
       // not connected
       BOOST_ASSERT( !my->m_session );
       
       my->hostname = host;
       my->uname    = user;
       my->port     = port;

       my->connect();

  }

  void client::connect( const std::string& user, const std::string& pass, const std::string& host, uint16_t port ) {

       my->hostname = host;
       my->uname    = user;
       my->upass    =  pass;
       my->port     = port;

       my->connect();
  }

  process::ptr client::exec( const std::string& cmd, const std::string& pty_type ) {
    process::ptr cpp(new process(*this,cmd,pty_type));
    return cpp;
  }

  process::ptr client::shell( const std::string& pty_type ) {
    process::ptr cpp(new process(*this,std::string(),pty_type));
    return cpp;
  }
  
  void client::rm( const boost::filesystem::path& remote_path ) {
    auto s = stat(remote_path);
    if( s.is_directory() ) {
      MACE_SSH_THROW( "Directory exists at path %1%", %remote_path );
    }
    else if( !s.exists() ) {
      return; // nothing to do
    }

    int rc = libssh2_sftp_unlink(my->m_sftp, remote_path.string().c_str() );
    while( rc == LIBSSH2_ERROR_EAGAIN ) {
      my->wait_on_socket();
      rc = libssh2_sftp_unlink(my->m_sftp, remote_path.string().c_str() );
    }
    if( 0 != rc ) {
       rc = libssh2_sftp_last_error(my->m_sftp);
       MACE_SSH_THROW( "rm error %1%", %rc );
    }
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
  void  client::scp_send( const boost::filesystem::path& local_path, const boost::filesystem::path& remote_path, 
                     boost::function<bool(size_t,size_t)> progress  ) {
    /**
     *  Tests have shown that if one scp is 'blocked' by a need to read (presumably to 
     *  ack recv for the trx window), and then a second transfer begins that the first
     *  transfer will never be acked.   Placing this mutex limits the transfer of
     *  one file at a time via SCP which is just as well because there is a fixed
     *  amount of bandwidth.  
     */
    boost::unique_lock<mace::cmt::mutex> lock(my->scp_send_mutex);


    using namespace boost::filesystem;
    if( !exists(local_path) ) {
      MACE_SSH_THROW( "Source file '%1%' does not exist", %local_path );
    }
    if( is_directory( local_path ) ) {
      MACE_SSH_THROW( "Source path '%1%' is a directory, expected a file.", %local_path );
    }

    using namespace boost::interprocess;
    // memory map the file
    file_mapping fmap( local_path.string().c_str(), read_only );
    size_t       fsize = file_size(local_path);

    mapped_region mr( fmap, boost::interprocess::read_only, 0, fsize );

    LIBSSH2_CHANNEL*                      chan = 0;
    time_t now;
    memset( &now, 0, sizeof(now) );
    // TODO: preserve creation / modification date
    chan = libssh2_scp_send64( my->m_session, remote_path.string().c_str(), 0700, fsize, now, now );
    while( chan == 0 ) {
      char* msg;
      int ec = libssh2_session_last_error( my->m_session, &msg, 0, 0 );
      if( ec == LIBSSH2_ERROR_EAGAIN ) {
        my->wait_on_socket();
        chan = libssh2_scp_send64( my->m_session, local_path.string().c_str(), 0700, fsize, now, now );
      } else {
          MACE_SSH_THROW( "scp %3% to %4% failed %1% - %2%", %ec %msg %local_path %remote_path );
      }
    }
    try {
      uint64_t   wrote = 0;
      char* pos = reinterpret_cast<char*>(mr.get_address());
      while( progress( wrote, fsize ) && wrote < fsize ) {
          int r = libssh2_channel_write( chan, pos, fsize - wrote );
          while( r == LIBSSH2_ERROR_EAGAIN ) {
            my->wait_on_socket();
            r = libssh2_channel_write( chan, pos, fsize - wrote );
          }
          if( r < 0 ) {
             char* msg = 0;
             int ec = libssh2_session_last_error( my->m_session, &msg, 0, 0 );
             MACE_SSH_THROW( "scp failed %1% - %2%", %ec %msg );
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
    if( ec < 0 ) {
       char* msg = 0;
       int ec = libssh2_session_last_error( my->m_session, &msg, 0, 0 );
       MACE_SSH_THROW( "scp failed %1% - %2%", %ec %msg );
    }
  }

  file_attrib client::stat( const boost::filesystem::path& remote_path ) {
     my->init_sftp();
     LIBSSH2_SFTP_ATTRIBUTES att;
     int ec = libssh2_sftp_stat( my->m_sftp, remote_path.string().c_str(), &att );
     while( ec == LIBSSH2_ERROR_EAGAIN ) {
        my->wait_on_socket();
        ec = libssh2_sftp_stat( my->m_sftp, remote_path.string().c_str(), &att );
     }
     if( ec ) {
        return file_attrib();
     }
     file_attrib    ft;
     ft.size        = att.filesize;
     ft.permissions = att.permissions;
     return ft;
  }

  void client::mkdir( const boost::filesystem::path& rdir, int mode ) {
    auto s = stat(rdir);
    if( s.is_directory() ) return;
    else if( s.exists() ) {
      MACE_SSH_THROW( "Non directory exists at path %1%", %rdir );
    }

    int rc = libssh2_sftp_mkdir(my->m_sftp, rdir.string().c_str(), mode );
    while( rc == LIBSSH2_ERROR_EAGAIN ) {
      my->wait_on_socket();
      rc = libssh2_sftp_mkdir(my->m_sftp, rdir.string().c_str(), mode );
    }
    if( 0 != rc ) {
       rc = libssh2_sftp_last_error(my->m_sftp);
       MACE_SSH_THROW( "mkdir error %1%", %rc );
    }
  }

  /**
   * 
   *  @post all session and socket objects closed and freed.
   *  @post my->m_session = NULL
   */
  void client::close() {
    if( my->m_session ) {
       if( my->m_sftp ) {
         int ec = libssh2_sftp_shutdown(my->m_sftp);
         try {
             while( ec == LIBSSH2_ERROR_EAGAIN ) {
                my->wait_on_socket();
                ec = libssh2_sftp_shutdown(my->m_sftp);
             } 
         }catch(...){
          elog( "... caught error closing sftp session???" );
         }
         my->m_sftp = 0;
       }
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
       } catch ( ... ){
          elog( "... caught error freeing session???" );
          my->m_session = 0;
       }
       try {
         if( my->m_sock ) {
           slog( "closing socket" );
           my->m_sock->close();
         }
       } catch ( ... ){
          elog( "... caught error closing socket???" );
       }
       my->m_sock.reset(0);
       try {
        if( my->read_prom ) my->read_prom->wait();
       } catch ( ... ){
        wlog( "caught error waiting on read prom" );
       }
       try {
        if( my->write_prom ) my->write_prom->wait();
       } catch ( ... ){
        wlog( "caught error waiting on write prom" );
       }
    }
    wlog("my->m_session = %1%", my->m_session );
  }


}} // mace::ssh
