
namespace mace { namespace rpc { namespace http {

  class connection_private {
    public:
      connection_private():sock( boost::make_shared<mace::cmt::asio::tcp::socket>() ),connected(false){}

      mace::cmt::asio::tcp::socket::ptr sock;
      bool connected;

      void read_line( std::string& line ) {
        mace::cmt::asio::tcp::socket::iterator it(sock.get());
        mace::cmt::asio::tcp::socket::iterator end;
        while( itr != end ) {
          if( *itr == '\n' ) {
            ++itr; return;
          }
          if( *itr != '\r' )
              line += *itr;
          ++itr;
        }
      }

      reply parse_reply() {
        reply r;
        std::string line;
        my->read_line( line );
        boost::regex rhttp("HTTP/1.([01]) (\\d+) (.*)" );
        boost::smatch m;
        if( boost::regex_search( line, m, rhttp, boost::match_extra) ) {
            r.http_version_minor = boost::lexial_cast<int>(m[0].str());
            r.status = boost::lexial_cast<int>(m[1].str());
            r.status_msg = boost::m[2].str();
        }
        line.clear();
        uint32_t cl = 0;
        read_line(line) 
        while( line.size() ) {
          if( line[0] != ' ' && line[0] != '\t' ) {
            size_t pos = line.find(':');
            r.headers.push_back( header( line.substr( 0, pos ), line.substr(pos+1,line.size() ) ) );
            boost::trim(r.headers.back().value);
            boost::to_lower(r.headers.back().name );
            if( r.headers.back().name == "content-length" ) {
              cl = boost::lexial_cast<int>(r.headers.back().name);
            } else if( r.headers.back().name == "connection" ) {
              boost::to_lower(r.headers.back().value );
              r.keep_alive = (r.headers.back().value == "keep-alive" )
            }
          } else if( r.headers.size() ) {
            r.headers.back().value += line;
            boost::trim(r.headers.back().value);
          } else {
            MACE_RPC_THROW( "Error parsing HTTP response, white-space before header key." );
          }
          line.clear();
          read_line(line) 
        }
        if( cl ) {
          std::vector<char> con(cl); 
          sock->read(&con.front(),cl);
          r.content.insert( 0, &con.front(), cl );
        }
        if( !r.keep_alive ) sock->close();
        return r;
      }
  };

  connection::connection( const std::string& host, uint16_t port )
    my = new connection_private(host,port);

    my->sock->connect( ep );
  };

  connection::~connection() { delete my; }

  reply connection::send( const request& r ) {
    std::stringstream ss;
    ss << r.method <<" "<<r.uri<< " HTTP/"<<r.http_version_major<<"."<<r.http_version_minor<<"\r\n";
    ss << "Host: "<< my->host << ":" << my->port << "\r\n";
    for( uint32_t i = 0; i < r.headers.size(); ++i ) {
      ss << r.headers[i].name <<": "<<r.headers[i].value<<"\r\n";
    }
    if( r.keep_alive ) ss<<"connection: keep-alive\r\n";
    else ss<<"Connection: close\r\n";

    ss << "Content-Length: "<<r.content.size()<<"\r\n\r\n";
    ss << r.content;
    my->sock->write( s.c_str(), s.size() );

    return my->parse_reply();
  }

} } }
