#include <mace/cmt/process.hpp>
#include <mace/cmt/error.hpp>
#include <mace/cmt/asio.hpp>
#include <boost/process.hpp>

namespace mace { namespace cmt {

  namespace bp = boost::process;
  namespace io = boost::iostreams;

  namespace detail {
     /*
    class process_sink : public io::sink {
      public:
        struct category : io::sink::category, io::flushable_tag {};
        typedef char      type;

        process_sink( process_d& p ):m_process(p){}
    
        std::streamsize write( const char* s, std::streamsize n );
        void close();
        bool flush();
    
      private:
        process_d&      m_process;
    };
    */

    class process_source : public io::source {
      public:
        typedef char      type;

        process_source(  boost::shared_ptr<bp::pipe>& pi )
        :m_pi(pi){}

        std::streamsize read( char* s, std::streamsize n ) {
          if( !m_pi ) return -1;
          try {
              return mace::cmt::asio::read_some( *m_pi, boost::asio::buffer( s, n ) );
          } catch ( const boost::system::system_error& e ) {
            if( e.code() == boost::asio::error::eof )  
                return -1;
            throw;
          }
        }
      private:
        boost::shared_ptr<bp::pipe>& m_pi;
    };


    class process_d { 
      public:
      process_d() 
      :std_out(process_source(outp)),
       std_err(process_source(errp)),
       std_in( process_sink(*this) ),
       stat( mace::cmt::asio::default_io_service() ){}
        
      boost::shared_ptr<bp::child> child;
      boost::shared_ptr<bp::pipe>  outp;
      boost::shared_ptr<bp::pipe>  errp;
      boost::shared_ptr<bp::pipe>  inp;

      io::stream<process_source>   std_out;
      io::stream<process_source>   std_err;
      io::stream<process_sink>     std_in;

      bp::status                   stat;
      bp::context                  pctx;
    };

    
    std::streamsize process_sink::write( const char* s, std::streamsize n ) {
       if( !m_process.inp ) return -1;
       return mace::cmt::asio::write( *m_process.inp, boost::asio::const_buffers_1( s, n ) );
    }
    void process_sink::close() {
       if( m_process.inp )
           m_process.inp->close();
    }
    bool process_sink::flush() {
      return true;
    }

  } // namespace detail

  process::process(){}
  process::process( process&& p )
  :my(std::move(p.my)){}

  process& process::operator=(process&& p ) {
    std::swap( my, p.my );
    return *this;
  }

  process process::exec( const boost::filesystem::path& exe, std::vector<std::string>&& args, const boost::filesystem::path& work_dir, int opt  ) {
    process p;
    p.my = std::make_shared<detail::process_d>();

    p.my->pctx.work_dir = work_dir.native();
    if( opt&open_stdout)
        p.my->pctx.streams[boost::process::stdout_id] = bp::behavior::async_pipe();
    else 
        p.my->pctx.streams[boost::process::stdout_id] = bp::behavior::null();


    if( opt& open_stderr )
        p.my->pctx.streams[boost::process::stderr_id] = bp::behavior::async_pipe();
    else
        p.my->pctx.streams[boost::process::stderr_id] = bp::behavior::null();

    if( opt& open_stdout )
        p.my->pctx.streams[boost::process::stdin_id]  = bp::behavior::async_pipe();
    else
        p.my->pctx.streams[boost::process::stdin_id]  = bp::behavior::close();

    p.my->child.reset( new bp::child( bp::create_child( exe.native(), std::move(args), p.my->pctx ) ) );

    if( opt & open_stdout ) {
       bp::handle outh = p.my->child->get_handle( bp::stdout_id );
       p.my->outp.reset( new bp::pipe( cmt::asio::default_io_service(), outh.release() ) );
    }
    if( opt & open_stderr ) {
       bp::handle errh = p.my->child->get_handle( bp::stderr_id );
       p.my->errp.reset( new bp::pipe( cmt::asio::default_io_service(), errh.release() ) );
    }
    if( opt & open_stdin ) {
       bp::handle inh  = p.my->child->get_handle( bp::stdin_id );
       p.my->inp.reset(  new bp::pipe( cmt::asio::default_io_service(), inh.release()  ) );
    }
    return p;
  }
  process process::shell( const std::string& cmd, int opt ) {
    BOOST_ASSERT(!"shell() not implemented yet" );
    return process();
  }

  mace::cmt::future<int> process::result() {
    promise<int>::ptr p(new promise<int>("process::kill"));
    my->stat.async_wait(  my->child->get_id(), [=]( const boost::system::error_code& ec, int exit_code )
      {
        if( !ec ) {
            #ifdef BOOST_POSIX_API
            if( WIFEXITED(exit_code) )
                p->set_value(  WEXITSTATUS(exit_code) );
            else
                MACE_CMT_THROW( "process exited with signal %1%", %WTERMSIG(exit_code) );
            #else
            p->set_value(exit_code);
            #endif
         }
         else p->set_exception( boost::copy_exception( boost::system::system_error(ec) ) );
      });
    return p;
  }

  void process::kill() {
    my->child->terminate();
  }
  
  /**
   *  @brief returns a stream that writes to the process' stdin
   */
  boost::iostreams::stream<detail::process_sink>& process::in_stream() {
    return my->std_in;
  }
  /**
   *  @brief returns a stream that reads from the process' stdout
   */
  std::istream& process::out_stream() { 
    return my->std_out;
  }
  /**
   *  @brief returns a stream that reads from the process' stderr
   */
  std::istream& process::err_stream() {
    return my->std_err;
  }


} } // namespace mace::cmt
