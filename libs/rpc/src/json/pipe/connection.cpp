#include <mace/rpc/json/io.hpp>
#include <mace/rpc/pipe/detail/connection.hpp>
#include <mace/rpc/json/pipe/detail/connection.hpp>
#include <mace/rpc/json/message.hpp>
#include <boost/asio.hpp>

namespace mace { namespace rpc { namespace json { namespace pipe {  namespace detail {

connection::connection( mace::rpc::connection_base& s, std::istream& i, std::ostream& o )
:mace::rpc::pipe::detail::connection(s,i,o) {}

connection::~connection() {
   try { close(); }
   catch(...) { elog( "%1%", boost::current_exception_diagnostic_information() ); }
}

/**
 *  Convert the generic rpc::message into a json-rpc message
 */
void  connection::send_message( rpc::message&& m ) {
  boost::unique_lock<mace::cmt::mutex> lock(wmutex);
  std::stringstream ss;
  // TODO: normalize this with other connections
  // TODO: switch all json connections to use boost::iostreams::stream
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
          ss<<",\"message\":"; ss.write( &m.data.front(), m.data.size() );
        }
        ss<<'}';
    }
    std::string s = ss.str();
    assert(s.size());
    m_out.write( s.c_str(), s.size() );
    
    if( !m.err ) {
      m_out.write(&m.data.front(), m.data.size() ); 
    }
    m_out.write( "}", 1 );
  } else {
    ss<<'}';
    std::string s = ss.str();
    m_out.write( s.c_str(), s.size() );
  }
  m_out.flush();
  assert(m_out);
}

rpc::message connection::read_message() {
   std::istream_iterator<char> itr(m_in);
   std::istream_iterator<char> end;
     
   auto msg  = read_value(itr,end);
   if( msg.size() < 5 ) {
        wlog( "'%1%'", msg );
   }
   if( !msg.size() )  {
     BOOST_THROW_EXCEPTION( boost::system::system_error( boost::asio::error::eof ) );
   }
   return to_message( std::move(msg) );
}

} } } } } // mace::rpc::raw::pipe::deatil

