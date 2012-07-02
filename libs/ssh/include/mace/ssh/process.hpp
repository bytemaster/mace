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

  class process : public std::enable_shared_from_this<process> {
    public:
      typedef std::shared_ptr<process> ptr;

      ~process();

      int result();
      boost::iostreams::stream<detail::process_sink>& in_stream();
      std::istream& out_stream();
      std::istream& err_stream();
    private:
      friend class client;
      process( client& c, const std::string& cmd );
      detail::process_d* my;
  };
} }
#endif //  _MACE_SSH_PROCESS_HPP_
