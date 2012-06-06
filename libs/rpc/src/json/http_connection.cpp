#include <boost/network/protocol/http/client.hpp>
#include <mace/rpc/json/connection.hpp>
#include <mace/rpc/json/http_client.hpp>
#include <mace/cmt/asio.hpp>
#include <mace/rpc/json/http_connection.hpp>
#include <boost/algorithm/string.hpp>

namespace mace { namespace rpc { namespace json {

typedef boost::network::http::client http10_client;

// called from io_service 
void async_post_http_request( 
  const mace::rpc::json::connection::ptr& con,
  const boost::shared_ptr<http10_client>& c,
  const boost::shared_ptr<http10_client::request>& rq,
  const std::string& json_str,
  const std::string& req_id,
  const connection::pending_result::ptr& rs  = connection::pending_result::ptr() )
{
  try {
    http10_client::response resp = c->post(*rq,json_str);//"application/json",json_str);
      std::string rsp = boost::network::http::body(resp);
      boost::trim(rsp);
    std::cout <<"'";
    std::cout<< rsp;
    std::cout<<"'\n";
    if( rs ) {
      json::value v;
      json::from_string( rsp, v );
      if( req_id.size() && ( v.contains( "id" ) && std::string(v["id"]) != req_id) ) {
         rs->handle_error( 
            boost::copy_exception( mace::rpc::exception() 
                    << err_msg("Response ID did not match request") ) );
      }
    
      if( v.contains( "result" ) ) {
        rs->handle_result( *con, v["result"] );
      } else if( v.contains( "error" ) ) {
         json::value& err= v["error"];
         std::string msg = "";
         int64_t code = 0;
         std::string data = "";
         if( err.contains("message") ) msg = (std::string)err["message"];
         if( err.contains("code") ) code = (int64_t)err["code"];
         if( err.contains("data") ) data = (std::string)err["data"];

         rs->handle_error( boost::copy_exception( 
                  mace::rpc::json::error_object() 
                               << json::err_code(code)
                               << json::err_data(data)
                               << rpc::err_msg(msg) ) );
      }
    }
  } catch ( ... ) {
    if( rs ) rs->handle_error( boost::current_exception() );
  }
}

using namespace boost::network::http;

class http_connection_private {
  public:
      http_connection_private( const std::string& url )
      :m_url(url),m_client(new http10_client( boost::network::http::_cache_resolved = true))
      {
        // network::http::_openssl_certificate 
        // network::http::_openssl_verify_path 
      }

      std::map<std::string,std::string>                m_headers;
      std::string                                      m_url;
      boost::shared_ptr<http10_client>  m_client;
};


http_connection::http_connection( const std::string& url )
:mace::rpc::json::connection() {
  my = new http_connection_private(url);
}

http_connection::~http_connection() {
  delete my;
}


void http_connection::send( const json::value& msg, 
                 const connection::pending_result::ptr& pr ) {
    boost::shared_ptr<http10_client::request> req 
        = boost::make_shared<http10_client::request>(my->m_url);

    for( std::map<std::string,std::string>::const_iterator i = my->m_headers.begin(); 
         i != my->m_headers.end(); ++i ) {
      *req << boost::network::header(i->first,i->second);
    }
    std::string json_str;
    json::to_string( msg, json_str );

    mace::cmt::asio::default_io_service().post( 
      boost::bind( async_post_http_request, shared_from_this(),
                  my->m_client, req, json_str, std::string(msg["id"]), pr ) );
}

void http_connection::send( const json::value& msg ) {
    boost::shared_ptr<http10_client::request> req 
        = boost::make_shared<http10_client::request>(my->m_url);
    std::string json_str;
    json::to_string( msg, json_str );
    mace::cmt::asio::default_io_service().post( 
      boost::bind( async_post_http_request, 
                  shared_from_this(),
                  my->m_client, req, json_str, std::string(),
                  connection::pending_result::ptr() ) );
}
  
void http_connection::set_url( const std::string& url ){
}

void http_connection::set_http_method( const std::string& method ){
}
 // POST / GET 
void http_connection::set_path( const std::string& path ){
}

void http_connection::set_args( const std::string& args ){
}

void http_connection::set_header( const std::string key, const std::string& value ){
  my->m_headers[key] = value;
}

boost::optional<std::string> http_connection::get_header( const std::string& key ){
  std::map<std::string,std::string>::const_iterator itr = my->m_headers.begin();
  if( itr != my->m_headers.end() ) return itr->second;

  return boost::optional<std::string>();
}

} } }  // mace::rpc::json
