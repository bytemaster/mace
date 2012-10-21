#include <mace/rpc/pipe/detail/connection.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio.hpp>
#include <mace/rpc/error.hpp>
#include <mace/cmt/bind.hpp>
#include <mace/cmt/thread.hpp>

namespace mace { namespace rpc { namespace pipe { namespace detail { 

  connection::connection( mace::rpc::connection_base& cb, std::istream& in, std::ostream& out )
  :mace::rpc::detail::connection_base(cb), m_in(in),m_out(out),m_created_thread(mace::cmt::thread::current()) {
     if( &m_in == &std::cin ) {
       m_read_thread = mace::cmt::thread::create("cin.read_loop");
     } else {
       m_read_thread = &mace::cmt::thread::current();    
     }
     m_read_done = m_read_thread->async( [this](){read_loop();}, "rpc.pipe.read_loop" );
  }
  connection::~connection() {
     try { close(); }
     catch(...) { elog( "%1%", boost::current_exception_diagnostic_information() ); }
  }
  void connection::close() {
     // cancel currently pending read, if we can
     m_read_done.cancel();
<<<<<<< HEAD
=======
      
>>>>>>> master
     if( &m_in == &std::cin ) {
       // If I call quit, this will hang because cin will 'block' waiting
       // for input therefore it will never join the thread.
       //  m_read_thread->quit();
     }
  }
  void connection::read_loop( ) {
    try {
      while ( true ) {
        //m_created_thread.async( mace::cmt::bind( [this]( message&& m ) { handle( std::move(m) ); } , read_message() ) );
        handle( read_message() );
        mace::cmt::yield();
      }
    } catch ( const mace::cmt::error::task_canceled& ) {
      // do nothing... just exit
    } catch ( const boost::system::system_error& e ) {
      // TODO: should other errors be signaled via closed
      // supress eof error, this is expected and indicated by the closed signal
      if( e.code() != boost::asio::error::eof ) 
        BOOST_THROW_EXCEPTION(e);
    } catch ( ... ) {
      wlog( "connection closed: %1%", boost::current_exception_diagnostic_information() );
    }
    break_promises();

    self.closed(); // emit signal
  }

}}}}
