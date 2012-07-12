#ifndef _MACE_CMT_PROCESS_HPP_
#define _MACE_CMT_PROCESS_HPP_
#include <boost/filesystem/path.hpp>
#include <mace/cmt/future.hpp>
#include <mace/cmt/detail/process.hpp>

namespace mace { namespace cmt { 

  /**
   *  You can move, but not copy a process.
   *
   *  A process may be default constructed in which case it is 'invalid'.  
   *
   *  Use !
   */
  class process : public std::enable_shared_from_this<process>, boost::noncopyable {
    public:
      enum exec_opts {
        open_none   = 0,
        open_stdin  = 0x01, 
        open_stdout = 0x02, 
        open_stderr = 0x04,
        open_all    = open_stdin|open_stdout|open_stderr,
      };

      typedef std::shared_ptr<process> ptr;
      process();
      process(process&& p);
      process& operator=( process&& m );

      // test for valid process reference...
      bool operator!()const{ return !my;   }
      operator bool()const { return !!my;  }

      /**
       *  Return a new process executing the specified exe with the specified args.
       */
      static process exec( const boost::filesystem::path& exe, 
                           std::vector<std::string>&& args = std::vector<std::string>(), 
                           const boost::filesystem::path& work_dir = boost::filesystem::path("."),
                           int opt = open_all );

      static process exec( const boost::filesystem::path& exe, std::vector<std::string>&& args, int opt  ) {
        return exec( exe, std::move(args), boost::filesystem::path("."), opt );
      }

      static process exec( const boost::filesystem::path& exe, int opt ) {
        return exec( exe, std::vector<std::string>(), boost::filesystem::path("."), opt );
      }

      static process exec( const boost::filesystem::path& exe, const boost::filesystem::path& work_dir, int opt = open_all ) {
        return exec( exe, std::vector<std::string>(), work_dir, opt );
      }

      static process shell( const std::string& cmd, int opt = open_all );
      
      /**
       *  Blocks current fiber until the result is received.
       */
      cmt::future<int> result();

      /**
       *  Forcefully kills the process.
       */
      void kill();
      
      /**
       *  @brief returns a stream that writes to the process' stdin
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
      std::shared_ptr<detail::process_d> my;
       
  };

} } // namespace mace::cmt


#endif //  _MACE_CMT_PROCESS_HPP_
