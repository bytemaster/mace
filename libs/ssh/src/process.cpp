#include <mace/ssh/process.hpp>
#include "client_detail.hpp"

namespace mace { namespace ssh { 

  namespace detail {
    struct process_d {
      std::string                           command;
      std::weak_ptr<mace::ssh::client>      sshc;
      mace::cmt::promise<int>::ptr          result;
      LIBSSH2_CHANNEL*                      chan;

      process_d( mace::ssh::client& c, const std::string& cmd )
      :sshc(c.shared_from_this()) {
        chan = c.my->wait_on_read( [&](){ return libssh2_channel_open_session(c.my->m_session); }, (LIBSSH2_CHANNEL*)(0) );
        if( !chan ) {
           MACE_SSH_THROW( "libssh2_channel_open_session returned NULL" );
        }
        c.my->wait_on_read( [=](){return libssh2_channel_exec(chan,cmd.c_str()); }, LIBSSH2_ERROR_EAGAIN, "libssh2_channel_exec" );
      }
    };
  } // namespace detail


  process::process( client& c, const std::string& cmd )
  :my( new detail::process_d( c, cmd ) ){}

  process::~process() { delete my; }

  /**
   *  This method will block until the remote channel is closed before
   *  it can return the exit code.
   *
   *  @pre client has not been freed.
   */
  int process::result() {
    std::shared_ptr<mace::ssh::client> sshc(my->sshc);
    BOOST_ASSERT( sshc );
    sshc->my->wait_on_read( [=](){return libssh2_channel_wait_closed( my->chan ); }, LIBSSH2_ERROR_EAGAIN );
    return libssh2_channel_get_exit_status( my->chan );
  }


} } // namespace mace::ssh
