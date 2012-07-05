#ifndef _MACE_SSH_CLIENT_HPP_
#define _MACE_SSH_CLIENT_HPP_
#include <mace/ssh/process.hpp>
#include <boost/function.hpp>

namespace mace { namespace ssh {
  namespace detail {
    class client_d;
  };

  enum sftp_file_type {
    named_pipe = 0010000,
    directory  = 0040000,
    regular    = 0100000,
    symlink    = 0120000
  };


  enum sftp_file_mode {
    owner_mask    =    0000700,     /* RWX mask for owner */
    owner_read    =    0000400,     /* R for owner */
    owner_write   =    0000200,     /* W for owner */
    owner_exec    =    0000100,     /* X for owner */
    group_mask    =    0000070,     /* RWX mask for group */
    group_read    =    0000040,     /* R for group */
    group_write   =    0000020,     /* W for group */
    group_exec    =    0000010,     /* X for group */
    other_mask    =    0000007,     /* RWX mask for other */
    other_read    =    0000004,     /* R for other */
    other_write   =    0000002,     /* W for other */
    other_exec    =    0000001      /* X for other */
  };

  struct file_attrib {
    file_attrib();

    size_t   size;
    uint32_t uid;
    uint32_t gid;
    uint32_t permissions;
    uint32_t atime;
    uint32_t mtime;

    bool     exists();
    bool     is_file();
    bool     is_directory();
  };


  /**
   *  @brief Enables communication over ssh using libssh2.
   *
   *  Because the client creates other resources that depend upon
   *  it, it can only be created as a std::shared_ptr<client> (aka client::ptr) 
   *  via client::create();
   */
  class client : public std::enable_shared_from_this<client> {
    public:
      typedef std::shared_ptr<client> ptr;
      static client::ptr create();

      void connect( const std::string& user, const std::string& host, uint16_t port = 22);
      void connect( const std::string& user, const std::string& pass, const std::string& host, uint16_t port = 22);

      /**
       *  @brief execute command on remote machine
       *  @param req_pty - whether or not to request a PTY when executing this process, this is necessary 
       *          for interactive (non-buffered) IO with the remote process
       *
       */
      process::ptr exec( const std::string& cmd, bool req_pty = false );

      /**
       *  @brief upload a file to remote host
       *  @param progress a callback to report / cancel upload.  
       *         The callback takes two parameters, bytes sent and file size.  To continue the 
       *         transfer, the callback should return true.  To cancel the callback should return false.
       */
      void scp_send( const std::string& local_path, const std::string& remote_path, 
                              boost::function<bool(size_t,size_t)> progress = [](size_t,size_t){return true;} );


      file_attrib stat( const std::string& remote_path );
      void mkdir( const std::string& remote_dir, int mode = owner_read|owner_write|owner_exec );

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
