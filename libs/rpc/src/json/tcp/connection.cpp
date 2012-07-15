#include <mace/rpc/json/io.hpp>
#include <mace/rpc/tcp/detail/connection.hpp>
#include <mace/rpc/json/tcp/detail/connection.hpp>
#include <mace/rpc/json/message.hpp>

namespace mace { namespace rpc { namespace json { namespace tcp {  namespace detail {

connection::connection(mace::rpc::connection_base& cb)
:mace::rpc::tcp::detail::connection(cb){}

connection::connection(mace::rpc::connection_base& cb, const boost::asio::ip::tcp::endpoint& ep )
:mace::rpc::tcp::detail::connection(cb,ep) {}

connection::connection(mace::rpc::connection_base& cb, const mace::cmt::asio::tcp::socket::ptr& sock )
:mace::rpc::tcp::detail::connection(cb,sock) {}

/**
 *  Convert the generic rpc::message into a json-rpc message
 */
void  connection::send_message( rpc::message&& m ) {
  std::stringstream ss;
  ss<<'{';
//  ss<<"\"jsonrpc\":\"2.0\",";
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
          ss<<",\"message\":";
          ss.write( &m.data.front(), m.data.size() );
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
  auto msg = read_value(itr,end);
  if( !msg.size() ) {
     BOOST_THROW_EXCEPTION( boost::system::system_error( boost::asio::error::eof ) );
  }
  return mace::rpc::json::to_message( std::move(msg) );
}

} } } } } // mace::rpc::raw::tcp::deatil

