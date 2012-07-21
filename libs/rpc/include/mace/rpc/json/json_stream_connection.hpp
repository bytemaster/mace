#ifndef _MACE_RPC_JSON_STREAM_CONNECTION_HPP_
#define _MACE_RPC_JSON_STREAM_CONNECTION_HPP_
#include <mace/rpc/stream_connection.hpp>
#include <mace/rpc/json/io.hpp>
#include <mace/cmt/mutex.hpp>

namespace mace { namespace rpc { namespace json {

  /**
   *  Implements the necessary methods to send and receive json-rpc over 
   *  a pair of input and output streams.
   */
  template<typename IStream=std::istream, typename OStream=std::ostream>
  class stream_connection : public mace::rpc::stream_connection< mace::rpc::json::io, IStream,OStream,json::stream_connection<IStream,OStream> > {
    public:

      typedef std::shared_ptr<stream_connection>                ptr;
      typedef mace::rpc::stream_connection< mace::rpc::json::io, IStream,OStream,stream_connection<IStream,OStream> > base_type;
      friend class mace::rpc::stream_connection<mace::rpc::json::io,IStream,OStream,stream_connection<IStream,OStream> >;
      typedef IStream                                           istream_type;             
      typedef OStream                                           ostream_type;             
      typedef mace::rpc::json::io                               io_delegate_type;
      typedef mace::rpc::message                                message_type;

      stream_connection( istream_type& i, ostream_type& o, const char* read_thread_name = NULL )
      :base_type( *this, i, o, read_thread_name ) { }

      stream_connection(){ }

    private:
       /**
        * This method must be re-entrant, so only one thread may be writing
        * a message at a time.
        *
        * @pre connection open
        */
       void send( message_type&& m ) {
          boost::unique_lock<mace::cmt::mutex> lock(write_mutex);
          if( !this->is_open() ) { MACE_RPC_THROW( "Connection Closed" ); }

          this->out().write( "{", 1 ); 
          bool c = false;
          if( m.has_request_id() ) {
             const char rid[] = "\"id\":";
             this->out().write( rid, sizeof(rid)-1 );
             std::string rids = boost::lexical_cast<std::string>(m.get_request_id() );
             this->out().write( rids.c_str(), rids.size() );
             c = true;
          } 
          if( m.has_method_id() ) {
            if( c ) {
                const char mid[] = ",\"method\":";
                this->out().write( mid, sizeof(mid)-1 );
            } else {
                const char mid[] = "\"method\":";
                this->out().write( mid, sizeof(mid)-1 );
            }
            std::string estr = '"'+ mace::rpc::json::escape_string(m.get_method_id())+'"';
            this->out().write(estr.c_str(), estr.size() );
            c = true;
          }
          if( m.has_params() ) {
            if( c ) {
                const char mid[] = ",\"params\":";
                this->out().write( mid, sizeof(mid)-1 );
            } else {
                const char mid[] = "\"params\":";
                this->out().write( mid, sizeof(mid)-1 );
            }
            this->out().write( &m.get_params().front(), m.get_params().size() );
            c = true;
          }
          if( m.has_result() ) {
            if( c ) {
                const char mid[] = ",\"result\":";
                this->out().write( mid, sizeof(mid)-1 );
            } else {
                const char mid[] = "\"result\":";
                this->out().write( mid, sizeof(mid)-1 );
            }
            this->out().write( &m.get_result().front(), m.get_result().size() );
            c = true;
          }
          if( m.has_error() ) {
            if( c ) {
                const char mid[] = ",\"error\":\"{\"code\":";
                this->out().write( mid, sizeof(mid)-1 );
            } else {
                const char mid[] = "\"error\":\"{\"code\":";
                this->out().write( mid, sizeof(mid)-1 );
            }
            std::string cd = boost::lexical_cast<std::string>(m.get_error().code.value );
            this->out().write( cd.c_str(), cd.size() );
            if( m.get_error().message ) {
                const char msg[] =",\"message\":\"";
                this->out().write( msg, sizeof(msg)-1 );
                std::string estr = mace::rpc::json::escape_string( *m.get_error().message ) + '"';
                this->out().write(estr.c_str(), estr.size() );
            }
            this->out().write( "}", 1 );
            c = true;
          }
          this->out().write("}\n",2);
          this->out().flush();
          if( !this->out() ) {
            MACE_RPC_THROW( "Error writing to stream" );
          }
       }

       void handle_error( connection_error e, message_type&& m ) {
          elog( "%1%", e );
          //BOOST_ASSERT( !"not yet implemented!" );
       }

       /**
        * This method is only ever called by the read loop, so there
        * is no need to make it reentrant;
        */
       message_type read_message() {
          std::istream_iterator<char> itr(this->in());
          std::istream_iterator<char> end;
            
          auto msg  = read_value(itr,end);
          if( !msg.size() )  {
            BOOST_THROW_EXCEPTION( boost::system::system_error( boost::asio::error::eof ) );
          }
          BOOST_ASSERT( msg.front() == '{' );
          //BOOST_ASSERT( msg.back()  == '}' );
          
          error_collector ec; // TODO: Do something with parse errors
          auto mfields = read_key_vals( &msg.front()+1, &msg.back()-1, ec );
          
          auto id      = mfields.find( "id" );
          auto result  = mfields.find( "result" );
          auto method  = mfields.find( "method" );
          auto params  = mfields.find( "params" );
     //     auto jsonrpc = mfields.find( "jsonrpc" );
          auto error   = mfields.find( "error" );
          auto e        = mfields.end();
          rpc::message m;
          if( id != e ) {
            id->second.json_data.push_back('\0');
            char* s =&id->second.json_data.front();
            //uint64_t e = id->second.json_data.size();
            //m.set_request_id( value_cast<int64_t>( to_value( s, s + e, ec ) ) );
            m.set_request_id( boost::lexical_cast<rpc::message::request_id_type>( s ) );
          }
          if( params != e ) {
            m.set_params( std::move( params->second.json_data ) ); 
          }
          if( method != e ) {
            //TODO: make this faster.. m.set_method_id( inplace_unescape_string(method->second) );
            m.set_method_id( unescape_string(std::string(method->second.json_data.begin(),method->second.json_data.end())) );
          }
          if( result != e ) {
            m.set_result( std::move( result->second.json_data ) ); 
          }
          if( error != e ) {
            auto efields = read_key_vals( &error->second.json_data.front()+1, 
                                          &error->second.json_data.back(), ec );
            auto code    = efields.find( "code" );
            auto message = efields.find( "message" );
            std::string emessage;
            error_object eo;
            if( message != efields.end() ) {
              eo.message = unescape_string(
                  std::string(message->second.json_data.begin(),
                              message->second.json_data.end() ));
            }
            code->second.json_data.push_back('\0');
            eo.code.value = boost::lexical_cast<int32_t>(&code->second.json_data.front());
            m.set_error( std::move(eo) );
          }
          return m; 
       }

       mace::cmt::mutex write_mutex;
  };

} } } 
#include <mace/rpc/connection_base.ipp>

#endif // _MACE_RPC_JSON_STREAM_CONNECTION_HPP_
