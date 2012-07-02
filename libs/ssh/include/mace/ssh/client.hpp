#ifndef _MACE_SSH_CLIENT_HPP_
#define _MACE_SSH_CLIENT_HPP_
#include <mace/ssh/process.hpp>
#include <boost/function.hpp>

namespace mace { namespace ssh {
  namespace detail {
    class client_d;
  };

  class client : public std::enable_shared_from_this<client> {
    public:
      typedef std::shared_ptr<client> ptr;
      static client::ptr create();

      void connect( const std::string& user, const std::string& host, uint16_t port = 22);

      process::ptr exec( const std::string& cmd );
      void         scp_send( const std::string& local_path, const std::string& remote_path, 
                              boost::function<bool(size_t,size_t)> progress  );

      ~client();

      void close();
    private:
      client();
      friend class process;
      friend class detail::process_d;
      detail::client_d* my;
  };

} } // namespace mace::ssh
#endif // _MACE_SSH_CLIENT_HPP_
