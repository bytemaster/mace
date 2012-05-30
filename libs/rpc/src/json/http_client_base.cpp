#include <mace/rpc/json/http_client_base.hpp>
#include <mace/rpc/json/http_connection.hpp>

namespace mace { namespace rpc { namespace json {

http_client_base::http_client_base( const std::string& url )
:client_base( boost::make_shared<http_connection>(url) )
{
}

void http_client_base::set_url( const std::string& url ){
    boost::dynamic_pointer_cast<http_connection>(m_con)->set_url(url);
}

void http_client_base::set_http_method( const std::string& method ){
    boost::dynamic_pointer_cast<http_connection>(m_con)->set_http_method(method);
}
 // POST / GET 
void http_client_base::set_path( const std::string& path ){
    boost::dynamic_pointer_cast<http_connection>(m_con)->set_path(path);
}

void http_client_base::set_args( const std::string& args ){
    boost::dynamic_pointer_cast<http_connection>(m_con)->set_args(args);
}

void http_client_base::set_header( const std::string key, const std::string& value ){
    boost::dynamic_pointer_cast<http_connection>(m_con)->set_header(key,value);
}

boost::optional<std::string> http_client_base::get_header( const std::string& key ){
    return boost::dynamic_pointer_cast<http_connection>(m_con)->get_header(key);
}
} } } // mace::rpc::json
