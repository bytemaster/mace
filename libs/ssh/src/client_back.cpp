#include <mace/cmt/asio/tcp/socket.hpp>
#include <mace/ssh/client.hpp>
#include <libssh2.h>

namespace mace { namespace ssh {
  namespace detail { 
    struct channel {
      typedef std::shared_ptr<channel> ptr;

    };

    struct client : public mace::cmt::asio::tcp::socket {
        client()
        :m_session(0),m_knownhosts(0)
        { }

        LIBSSH2_SESSION *    m_session;
        LIBSSH2_KNOWNHOSTS * m_knownhosts;

        mace::cmt::ssh::client::key host_key;
        std::string                 uname;
        std::string                 upass;
        std::string                 pubkey;
        std::string                 privkey;
        std::string                 hostname;
        uint16_t                    port;
        bool                        session_connected;

        std::vector<mace::cmt::ssh::client::auth_methods> avail_auth_methods;
        std::vector<mace::cmt::ssh::client::auth_methods> failed_auth_methods;
        std::list<channel::ptr>                           channels;

        void reset() {
          if( m_knownhosts ) {
            libssh2_knownhost_free(m_knownhosts);
          }
          if( session_connected ) { 
            libssh2_session_disconnect(m_session, "good bye!" );
          }
          if( m_session ) { libssh2_session_free(m_session); }

          // close the socket...
          close();

          session_connected = false;
          failed_auth_mehtods.clear();
          avail_auth_methods.clear();

          m_session = libssh2_session_init_ex(NULL,NULL,NULL,reinterpret_cast<void*>(this)); 
          BOOST_ASSERT( m_session );
          
          m_knownhosts = libssh2_knownhost_init( m_session );
          BOOST_ASSERT( m_knownhosts );

          /**
           *  We want to use non blocking, so that when we get EAGAIN we can ask
           *  ASIO to block.  In this way we never yield 'within' libssh2 library
           *  and we can truly multiplex.
           */
          libssh2_session_set_blocking(d_session,0);
        }

        bool validate_remote_host() {
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

        bool try_auth() {
            char * alist=libssh2_userauth_list(m_session, uname.c_str(),uname.size());
            if(alist==NULL){
                if(libssh2_userauth_authenticated(m_session)){
                    // CONNECTED!
                    return;
                }else if(libssh2_session_last_error(m_session,NULL,NULL,0)==LIBSSH2_ERROR_EAGAIN) {
                    MACE_CMT_THROW( "LIBSSH2_ERROR_EAGAIN" );
                }else{
                    reset();
                    MACE_CMT_THROW( "libssh2 unexpected shutdown error" );
                }
            }

            std::vector<std::string> split_alist;
            bool pubkey = false;
            bool pass   = false;
            boost::split( split_alist, alist, boost::is_any_of(",") );
            std::for_each( split_alist.begin(), split_alist.end(), [&](const std::string& s){
              if( s == "publickey" ) {
                pubkey = true;
              }
              if( s == "password" ) {
                pass = true;
              }
            });

            if( pubkey ) {
              if( try_pub_key() ) 
                return true;
            }
            if( pass ) {
              if( try_pass() )
                return true;
            }
            MACE_CMT_THROW( "Unable to authenticate" );
            return false;
        }

        bool try_pass() {
          return 0==libssh2_userauth_password(m_session, uname.c_str(), pass.c_str() );
        }

        bool try_pub_key() {
          return 0==libssh2_userauth_publickey_fromfile(m_session,
                                                       uname.c_str(),
                                                       pubkey.c_str(),
                                                       privkey.c_str(),
                                                       passphrase.c_str() );
        }

    }; // detail::client


    struct process {


      /**
       *  Will return as much data as available up to the buffer size.
       *  Waits for at least 1 byte.
       */
      size_t read_some( char* buf, size_t len, mace::cmt::ssh::client::process::channel_num chan ) {
        ssize_t ret = libssh2_channel_read_ex(m_channel, chan, buf, len );
        if( ret < 0 ) {
          MACE_CMT_THROW( "libssh2_channel_read_ex returned %1%", %ret );
        }
        return ret;
      }

      /**
       *  Will not return until len bytes has been read.
       */
      size_t read( char* buf, size_t len, mace::cmt::ssh::client::process::channel_num chan ) {
        size_t total = 0;
        while( total < len ) {
            ssize_t ret = libssh2_channel_read_ex(m_channel, chan, buf+total, len-total );
            if( ret < 0 ) {
              MACE_CMT_THROW( "libssh2_channel_read_ex returned %1%", %ret );
            }
            total += ret;
        }
        return total;
      }
      size_t write( const char* buf, size_t len, mace::cmt::ssh::client::process::channel_num chan ) {
        ssize_t ret = libssh2_channel_write_ex(m_channel, chan, buf, len );
        if( ret < 0 ) {
          MACE_CMT_THROW( "libssh2_channel_write_ex returned %1%", %ret );
        }
        return ret;
      }
      
      std::string                           command;
      std::weak_ptr<mace::cmt::ssh::client> sshc;
    };



  } // namespace detail

  client::ptr client::create() {
    return std::make_shared<client>();
  }

  client::client() {
    my = new detail::client();
  }

  client::~client() {
    delete my;
  }


  /**
   *  Connect and authenticate or throw.
   */
  void client::connect_to_host( const std::string& user, const std::string& host, int port ) {
    my->hostname = host;
    my->uname    = user;
    my->port     = port;
    my->state    = 1;
    
    if( !libssh2_init(0) ) {
      MACE_CMT_THROW( "Unable to init libssh2" );
    }
    my->reset();

    boost::system::error_code e;
    if( e = my->connect( host, port ) ) {
       BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
    }
    my->session_connected = true;
    
    // init ssh session, exchange banner etc
    int ret = 0; 
    if((ret = libssh2_session_startup(m_session, my->native())) ==LIBSSH2_ERROR_EAGAIN){
        MACE_CMT_THROW( "LIBSSH2_ERROR_EAGAIN" );
    }
    if( ret ) {
        reset();
        MACE_CMT_THROW( "Failure establishing SSH session: %1%", ret );
    }

    // make sure remote is safe
    my->validate_remote_host();
    // attempt auth
    my->try_auth();

    return e;
  }

  client::process::ptr client::exec( const std::string& cmd ) {
    auto cpp = std::make_shared<client::process>(std::ref(*this), cmd);
    my->processes.push_back(ccp);
    return cpp;
  }


}} // mace::ssh
