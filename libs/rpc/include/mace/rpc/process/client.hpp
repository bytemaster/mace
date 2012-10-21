#ifndef  _MACE_RPC_PROCESS_CLIENT_HPP_
#define  _MACE_RPC_PROCESS_CLIENT_HPP_
#include <mace/stub/ptr.hpp>
#include <mace/rpc/client_interface.hpp>
#include <mace/cmt/process.hpp>
#include <mace/rpc/error.hpp>
#include <boost/signals.hpp>

namespace mace { namespace rpc {  namespace process {
  enum options_enum {
    recv_err_stream =  0x01, // receive stderr from process
    recv_out_stream =  0x02, // receive stdout from process
    use_out_stream  =  0x04 | recv_out_stream, // use stdout as in RPC reply channel, implies recv_out_stream
    use_err_stream  =  0x08 | recv_err_stream  // use stderr as the RPC reply channel, implies recv_err_stream
    // TODO: can we support using both streams for RPC?
  };

  /**
   *  This class has 'pointer semantics', copies refer to the same process.
   *
   *  @tparam ConnectionType - must impliment mace::rpc::pipe::connection<T> interface
   */
  template<typename InterfaceType, typename ConnectionType >
  class client : public mace::stub::ptr<InterfaceType, mace::rpc::client_interface< ConnectionType > > {
    public:
      typedef mace::rpc::client_interface< ConnectionType > delegate_type;
      typedef ConnectionType				    connection_type;

      client( const typename ConnectionType::ptr& c )
      :m_opt(0),m_con(c){
        delegate_type::set_vtable( *this, m_con );
      }
      template<typename IStream, typename OStream>
      client( IStream& in, OStream& out )
      :m_opt(0),m_con( new ConnectionType( in, out ) ){ }

      client():m_opt(0){}

      client( const client& c ):m_con(c.m_con) {
         delegate_type::set_vtable( *this, m_con );
      }
      client( client&& c ) {
        m_opt = c.m_opt;
        m_proc = std::move(c.m_proc);
        m_con = std::move(c.m_con);
      }

      void close() {
        //if( m_proc ) m_proc->in_stream().close();
        if( m_con  ) m_con->close();
      }

      void kill() {
        if( m_proc ) m_proc->kill();
      }

      /**
       *  Starts the process in the specified working directory with the given exe and args.
       *
       *  @pre work_dir exists
       *  @pre has not been called before.
       *  @pre exe exists and is either absolute or relative to work_dir
       *  @pre exe expects RPC over stdin/stdout 
       *
       *  @param args passed as rvalue ref to force the caller to make copies explicit.
       *  @param opt  used to specify which output channels to use for RPC and whether or not
       *              to receive the other channels.
       */
      void  exec( const boost::filesystem::path& exe, std::vector<std::string>&& args = std::vector<std::string>(), 
                  const boost::filesystem::path& work_dir=".", int opt = use_out_stream | recv_err_stream) {
        BOOST_ASSERT( !m_proc );
        BOOST_ASSERT( !m_con  );

        m_opt = opt;
        int opts = mace::cmt::process::open_stdin;
        if( recv_out_stream & opt ) opts |= mace::cmt::process::open_stdout;
        if( recv_err_stream & opt ) opts |= mace::cmt::process::open_stderr;

        m_proc.reset( new mace::cmt::process( mace::cmt::process::exec( exe, std::move(args), work_dir, opts ) ) );

        if( use_out_stream & opt )
            m_con.reset( new ConnectionType( m_proc->out_stream(), m_proc->in_stream(), "rpc_read_loop" ) );
        else if( use_err_stream & opt ) {
            m_con.reset( new ConnectionType( m_proc->err_stream(), m_proc->in_stream(), "rpc_read_loop" ) );
        }
        delegate_type::set_vtable( *this, m_con );
      }
      void  exec( const boost::filesystem::path& exe, const boost::filesystem::path& work_dir, int opt = use_out_stream | recv_err_stream) {
        exec( exe, std::vector<std::string>(), work_dir, opt );
      }
      void  exec( const boost::filesystem::path& exe, std::vector<std::string>&& args,  int opt ) {
        exec( exe, std::move(args), ".", opt );
      }


      bool operator!()const         { return !m_con; }
      int  get_start_options()const { return m_opt; }

      /**
       *  @return the connection used by this client.
       */
      typename ConnectionType::ptr connection()const { return m_con; }

      /**
       *  @pre process has been started.
       */
      int result()const { BOOST_ASSERT(m_proc); return m_proc->result(); }

      /**
       *  Returns the error stream for this process 
       *
       *  @throw if process was started without recv_err_stream
       *  @throw if err stream is being used for RPC coms
       */
      std::istream&              err_stream() { 
        BOOST_ASSERT( m_proc );
        if( !(m_opt & use_out_stream ) ) MACE_RPC_THROW( "stderr is in use by RPC connection" );
        if( !(m_opt & recv_err_stream) ) MACE_RPC_THROW( "no err stream available" );
        return m_proc->err_stream(); 
      }

      /**
       *  Returns the out stream for this process 
       *
       *  @throw if process was started with without recv_out_stream
       *  @throw if out stream is being used for RPC coms
       */
      std::istream&              out_stream() { 
        BOOST_ASSERT( m_proc );
        if( !(m_opt & use_err_stream ) ) MACE_RPC_THROW( "stdout is in use by RPC connection" );
        if( !(m_opt & recv_out_stream) ) MACE_RPC_THROW( "no output stream available" );
        return m_proc->out_stream(); 
      }

    private:
      int                            m_opt;
      mace::cmt::process::ptr        m_proc;
      typename ConnectionType::ptr   m_con;
  };

} } }// namespace mace::rpc::process

#endif // _MACE_RPC_PROCESS_CLIENT_HPP_
