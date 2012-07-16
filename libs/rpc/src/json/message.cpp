#include <mace/rpc/json/message.hpp>
#include <mace/rpc/json/error_collector.hpp>
#include <mace/rpc/json/io.hpp>

namespace mace { namespace rpc { namespace json {

  /**
   */
  mace::rpc::message to_message( datavec&& msg ) {
      slog( "%1%", std::string( msg.begin(), msg.end() ) );
      BOOST_ASSERT( msg.front() == '{' );
      //BOOST_ASSERT( msg.back()  == '}' );

      error_collector ec; // TODO: Do something with parse errors
      auto mfields = read_key_vals( &msg.front()+1, &msg.back()-1, ec );

      auto id      = mfields.find( "id" );
      auto result  = mfields.find( "result" );
      auto method  = mfields.find( "method" );
      auto params  = mfields.find( "params" );
      auto jsonrpc = mfields.find( "jsonrpc" );
      auto error   = mfields.find( "error" );
      auto e        = mfields.end();
      rpc::message m;
      if( id != e ) {
        char* s = const_cast<char*>(id->second.json_data.c_str());
        uint64_t e = id->second.json_data.size();
        m.id = value_cast<int64_t>( to_value( s, s + e, ec ) );
      }
      if( params != e ) {
        m.data = datavec(params->second.json_data.begin(),params->second.json_data.end() );
      }
      if( method != e ) {
        m.meth = unescape_string(method->second.json_data);
      }
      if( result != e ) {
        m.data = datavec(result->second.json_data.begin(),result->second.json_data.end() );
      }
      if( error != e ) {
        auto efields = read_key_vals( &error->second.json_data.front()+1, 
                                      &error->second.json_data.back(), ec );
        auto code    = efields.find( "code" );
        auto message = efields.find( "message" );
        std::string emessage;
        if( message != efields.end() ) {
          emessage = unescape_string(message->second.json_data);
        }
        m.err    =  static_cast<mace::rpc::message::error_type>(boost::lexical_cast<int>(code->second.json_data));
        m.data = datavec(emessage.begin(),emessage.end());
      }
      return m; 
  }

} } } 
