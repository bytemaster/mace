#include <mace/cmt/asio/tcp/socket.hpp>
#include <mace/ssh/client.hpp>
#include <mace/ssh/error.hpp>
#include <mace/cmt/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "client_detail.hpp"

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

        bool client_d::try_auth() {
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

        bool client_d::try_pass() {
          return 0==libssh2_userauth_password(m_session, uname.c_str(), pass.c_str() );
        }

        bool client_d::try_pub_key() {
          return 0==libssh2_userauth_publickey_fromfile(m_session,
                                                       uname.c_str(),
                                                       pubkey.c_str(),
                                                       privkey.c_str(),
                                                       passphrase.c_str() );
        }
        */

  } // namespace detail

  client::ptr client::create() {
    client::ptr c = client::ptr(new client());
    return c;
  }

  client::client() {
    my = new detail::client_d(*this);
  }

  client::~client() {
    delete my;
  }


  /**
   *  Connect and authenticate or throw.
   */
  void client::connect( const std::string& user, const std::string& host, uint16_t port ) {
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

    slog( "creating socket" );
    my->m_sock.reset( new boost::asio::ip::tcp::socket( mace::cmt::asio::default_io_service() ) );

    for( uint32_t i = 0; i < eps.size(); ++i ) {
       boost::system::error_code e = mace::cmt::asio::tcp::connect( *my->m_sock, eps[i] );
       if( !e ) {
         slog( "connected to remote endpoint" );
         break;
       }
    }
    slog( "Creating session" );
    my->m_session = libssh2_session_init(); 
    libssh2_session_set_blocking( my->m_session, 1 );
    BOOST_ASSERT( my->m_session );

      char* msg;
    libssh2_session_last_error( my->m_session, &msg, 0, 0 );
    elog( "before startup %1%",msg);

    int rtn = libssh2_session_handshake( my->m_session, my->m_sock->native() );
    libssh2_session_last_error( my->m_session, &msg, 0, 0 );
    elog( "before startup %1%",msg);

    slog( "starting session" );
    rtn = libssh2_session_startup(my->m_session, my->m_sock->native());
    if( rtn < 0 ) {
      libssh2_session_last_error( my->m_session, &msg, 0, 0 );
      elog( "%1%",msg);
    }
    slog( "rtn : %1% vs EAGAIN:%2%", rtn, LIBSSH2_ERROR_EAGAIN );
    my->wait_on_read( [&]() {
      return libssh2_session_startup(my->m_session, my->m_sock->native()); }, LIBSSH2_ERROR_EAGAIN );
    slog( "session started" );
    /*
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
    */
  }

  process::ptr client::exec( const std::string& cmd ) {
    process::ptr cpp(new process(*this,cmd));
    //my->processes.push_back(ccp);
    return cpp;
  }


}} // mace::ssh
