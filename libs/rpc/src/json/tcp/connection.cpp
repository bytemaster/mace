#include <mace/rpc/json/io.hpp>
#include <mace/rpc/tcp/detail/connection.hpp>
#include <mace/rpc/json/tcp/detail/connection.hpp>
#include <mace/rpc/raw/message.hpp>

namespace mace { namespace rpc { namespace json { namespace tcp {  namespace detail {

connection::connection(){}

connection::connection( const boost::asio::ip::tcp::endpoint& ep )
:mace::rpc::tcp::detail::connection(ep) {}

connection::connection( const mace::cmt::asio::tcp::socket::ptr& sock )
:mace::rpc::tcp::detail::connection(sock) {}

/**
 *  Convert the generic rpc::message into a json-rpc message
 */
void  connection::send_message( rpc::message&& m ) {
  std::stringstream ss;
  ss<<'{';
  ss<<"\"jsonrpc\":\"2.0\",";
  bool c = false;
  if( m.id ) { ss<<"\"id\":"<<*m.id; c = true; }
  if( m.meth.size() ) {
    if( c ) ss<<',';
    ss<<"\"method\":\""<<mace::rpc::json::escape_string(m.meth)<<'"';
    c = true;
  }
  if( m.data.size() ) {
    if( c ) ss<<',';
    if( m.meth.size() ) {
        ss<<"\"params\":";
    }
    else if( m.err == message::none ) {
        ss<<"\"result\":";
    } else {
        ss<<"\"error\":{\"code\":";
        ss<<int(m.err);
        if( m.data.size() ) {
          ss<<",\"message\":\"";
          ss.write( &m.data.front(), m.data.size() );
          ss<<'"';
        }
        ss<<'}';
    }
    std::string s = ss.str();
    m_sock->write( s.c_str(), s.size() );
    if( !m.err )
        m_sock->write( &m.data.front(), m.data.size() );
    m_sock->write( "}", 1 );
  } else {
    ss<<'}';
    std::string s = ss.str();
    m_sock->write( s.c_str(), s.size() );
  }
}
rpc::message connection::read_message() {
  mace::cmt::asio::tcp::socket::iterator itr(m_sock.get());
  mace::cmt::asio::tcp::socket::iterator end;
  std::vector<char> msg = read_value(itr,end);
  if( !msg.size() ) return rpc::message();

  error_collector ec;
  std::map<std::string,json::string> mfields = read_key_vals( &msg.front()+1, &msg.front() + msg.size()-2,ec );
  auto id = mfields.find( "id" );
  auto result = mfields.find( "result" );
  auto method = mfields.find( "method" );
  auto params = mfields.find( "params" );
  auto jsonrpc = mfields.find( "jsonrpc" );
  auto error = mfields.find( "error" );
  auto e   = mfields.end();
  rpc::message m;
  if( id != e ) {
    char* s = const_cast<char*>(id->second.json_data.c_str());
    uint32_t e = id->second.json_data.size();
    m.id = value_cast<int32_t>( to_value( s, s + e, ec ) );
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
    m.data = datavec(error->second.json_data.begin(),result->second.json_data.end() );
  }
  return m; 
}

} } } } } // mace::rpc::raw::tcp::deatil

