
namespace mace { namespace rpc { namespace json {


http_client_base::http_client_base( const std::string& url ) {
  client_base::m_con = boost::make_shared<http_connection>(url);
  slog( "m_con: %1%", m_con.get() );
}


} } } // mace::rpc::json
