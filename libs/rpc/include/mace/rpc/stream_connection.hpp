#ifndef _MACE_RPC_STREAM_CONNECTION_HPP_
#define _MACE_RPC_STREAM_CONNECTION_HPP_
#include <mace/rpc/connection.hpp>
#include <mace/cmt/thread.hpp>
#include <mace/cmt/bind.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/error.hpp>

namespace mace { namespace rpc {
   namespace detail {
      template<typename T>
      struct has_close {
        typedef char (&no_tag)[1];
        typedef char (&yes_tag)[2];

        template<typename C, void (C::*)() > struct has_close_helper{};

        template<typename C >
        static no_tag has_member_helper(...);

        template<typename C>
        static yes_tag has_member_helper( has_close_helper<C,&C::close>* p);
        
        BOOST_STATIC_CONSTANT( bool, value = sizeof(has_member_helper<T>(0)) == sizeof(yes_tag) );
      };

      template<typename C, bool HasClose = has_close<C>::value>
      struct close { void operator()( C& c ) { c.close(); } };

      template<typename C>
      struct close<C,false> { void operator()( C& ) { } };
   }



  /**
   *  
   *  @tparam Derived implements:
   *    
   *    typedef IStream           istream_type
   *    typedef OStream           ostream_type
   *    typedef Message           message_type;
   *    typedef IODelegate        io_delegate_type;
   *    void send( message_type&& m )
   *    void handle_error( connection_error_enum, message_type&& m )
   *    message_type read_message()
   *
   */
  template<typename IODelegate, typename IStream, typename OStream, typename Derived>
  class stream_connection : public connection< stream_connection<IODelegate,IStream,OStream,Derived> > {
    public:
      typedef IODelegate io_delegate_type;

      friend class connection< stream_connection<IODelegate,IStream,OStream,Derived> >;

      // forward to derived class
      void send( message&& m ) { static_cast<Derived*>(this)->send( std::move(m) ); }
      void handle_error( connection_error e, message&& m ) { 
        static_cast<Derived*>(this)->handle_error( e, std::move(m) ); 
      }

      void close() {
        if( _open ) {
           // try to close the ostream if the type supports the close method.
           //try_close( *_out );
           detail::close<OStream> c;
           c(*_out);

           _open = false;
           _read_done.cancel();
        }
      }

      typedef IStream istream_type;
      typedef OStream ostream_type;
      ~stream_connection() {
         try { close(); }
         catch(...) { elog( "%1%", boost::current_exception_diagnostic_information() ); }
      }

      bool connected() { return _open; }

      void open( istream_type& i, ostream_type& o, const char* read_thread_name = NULL ) {
        _open = true;
        _in  = &i;
        _out = &o;
        _handle_thread = &mace::cmt::thread::current();
        if( read_thread_name ) {
          _read_thread = mace::cmt::thread::create( read_thread_name );
        } else {
          _read_thread = _handle_thread;
        }
        _read_done = _read_thread->async( [this](){this->read_loop();}, "rpc::stream_connection::read_loop" );
      }

      bool is_open()const { return _open; }

    protected:
      stream_connection( Derived& s )
      :_derived_self(s),_in(NULL),_out(NULL),_handle_thread(NULL),_read_thread(NULL){}

      /**
       *  @param some streams (std::cin) block, and therefore must be managed in
       *         a separate thread.
       */
      stream_connection( Derived& s, istream_type& i, ostream_type& o, const char* read_thread_name = NULL )
      :_derived_self(s) { 
        open( i, o, read_thread_name );
      }

      void read_loop( ) {
        try {
          while ( _open ) {
            _handle_thread->async( mace::cmt::bind( [this]( typename Derived::message_type&& m ) { this->handle( std::move(m) ); } , _derived_self.read_message() ) );
            mace::cmt::yield();
          }
        } catch ( const boost::system::system_error& e ) {
          // TODO: should other errors be signaled via closed
          // supress eof error, this is expected and indicated by the closed signal
          if( e.code() != boost::asio::error::eof ) 
              wlog( "connection closed: %1%", boost::current_exception_diagnostic_information() );
           else {
            wlog("%1%", boost::system::system_error(e).what() );
           }
        } catch ( const mace::cmt::error::task_canceled&  ) {
          // ignore this..
        } catch ( ... ) {
          wlog( "connection closed: %1%", boost::current_exception_diagnostic_information() );
        }
        _open = false;

        this->break_promises();
        this->closed(); // emit signal
        if( _read_thread ) _handle_thread->async( [this](){ this->closed(); } ).wait();
        else { this->closed(); }
      }
      istream_type& in() { BOOST_ASSERT( _in && _open );  return *_in;  }
      ostream_type& out(){ BOOST_ASSERT( _out && _open ); return *_out; }

    private:
      istream_type*           _in;
      ostream_type*           _out;

      // not all streams support the 'close' method, so we will only call close
      // if the stream supports it.
      template<typename OS>
      void try_close( OS& o,  void (OS::*close)() = &OS::close ) {
        slog( "Found close method on stream!" );
        o.close();
      }
      /// base case, should only match if OStream does not have a close method.
      template<typename OS>
      void try_close( OS& o, int x = 5 ) { wlog( "unable to find close method on stream" ); }

      Derived&                _derived_self;
      bool                    _open;
      mace::cmt::thread*      _read_thread;
      mace::cmt::future<void> _read_done; 
      mace::cmt::thread*      _handle_thread;
      stream_connection();
  };

} } 

#endif //_MACE_RPC_STREAM_CONNECTION_HPP_
