#ifndef _MACE_SSH_PROCESS_HPP_
#define _MACE_SSH_PROCESS_HPP_
#include <memory>
#include <boost/iostreams/stream.hpp>

namespace mace { namespace ssh {

    namespace detail {
    namespace io = boost::iostreams;

    class process_d;
      class process_sink : public io::sink {
        public:
          struct category : io::sink::category, io::flushable_tag {};
          typedef char      type;
      
          process_sink( process_d& p, int chan )
          :m_process(p),m_chan(chan){}
      
          std::streamsize write( const char* s, std::streamsize n );
          void close();
          bool flush();
      
        private:
          process_d&      m_process;
          int             m_chan;
      };
    } // namespace detail
  class client;

  /**
   *  Enables communication with a process executed via 
   *  client::exec().  
   *
   *  Process can only be created by mace::ssh::client.
   */
  class process : public std::enable_shared_from_this<process> {
    public:
      typedef std::shared_ptr<process> ptr;

      ~process();

      /**
       *  Blocks until the result code of the process has been returned.
       */
      int result();
      /**
       *  @brief returns a stream that writes to the procss' stdin
       */
      boost::iostreams::stream<detail::process_sink>& in_stream();
      /**
       *  @brief returns a stream that reads from the process' stdout
       */
      std::istream& out_stream();
      /**
       *  @brief returns a stream that reads from the process' stderr
       */
      std::istream& err_stream();
    private:
      friend class client;
      process( client& c, const std::string& cmd );
      detail::process_d* my;
  };
} }
#endif //  _MACE_SSH_PROCESS_HPP_
