#include <mace/rpc/json/io.hpp>
#include <mace/rpc/pipe/detail/connection.hpp>
#include <mace/rpc/json/pipe/detail/connection.hpp>
#include <mace/rpc/raw/message.hpp>
#include <boost/asio.hpp>

namespace mace { namespace rpc { namespace json { namespace pipe {  namespace detail {

connection::connection( mace::rpc::connection_base& s, std::istream& i, std::ostream& o )
:mace::rpc::pipe::detail::connection(s,i,o)
{}

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
    m_out.write( s.c_str(), s.size() );
//    std::cerr<<s;
    if( !m.err ) {
      m_out.write( &m.data.front(), m.data.size() );
//      std::cerr.write(&m.data.front(), m.data.size() );
    }
    m_out.write( "}", 1 );
//    std::cerr<<'}';
  } else {
    ss<<'}';
    std::string s = ss.str();
//    std::cerr<<s;
    m_out.write( s.c_str(), s.size() );
  }
  m_out.flush();
}
rpc::message connection::read_message() {
  std::vector<char> msg;
  {
//    boost::unique_lock<mace::cmt::mutex> lock(rmutex);
    std::istream_iterator<char> itr(m_in);
    std::istream_iterator<char> end;
    
    msg  = read_value(itr,end);
  }
//  slog( "%1%", std::string(msg.begin(),msg.end()) );
  if( !msg.size() )  {
    BOOST_THROW_EXCEPTION( boost::system::system_error( boost::asio::error::eof ) );
    //MACE_RPC_THROW( "EOF Message" );
  }

  error_collector ec;
  std::map<std::string,json::string> mfields = read_key_vals( &msg.front()+1, &msg.front() + msg.size()-2,ec );
  auto id = mfields.find( "id" );
  auto result = mfields.find( "result" );
  auto method = mfields.find( "method" );
  auto params = mfields.find( "params" );
  auto jsonrpc = mfields.find( "jsonrpc" );
  auto error = mfields.find( "error" );
  auto e   = mfields.end();
  bool valid = false;
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

} } } } } // mace::rpc::raw::pipe::deatil

