#include <boost/cmt/asio/tcp.hpp>
#include <boost/cmt/asio/tcp/socket.hpp>
#include <boost/cmt/thread.hpp>
#include <sstream>
#include <boost/exception/all.hpp>

void main2() {
try {
    boost::cmt::asio::tcp::socket sock;
    boost::system::error_code ec = sock.connect( boost::cmt::asio::tcp::resolve( "www.apple.com", "http" ).front() ); 
    if( ec ) {
        elog( "error connecting: %1%", boost::system::system_error(ec).what() );
        return;
    }
    std::stringstream i;
    i << "GET /index.html HTTP/1.0\r\n";
    i << "HOST: www.apple.com\r\n";
    i << "Accept: */*\r\n";
    i << "Connection: close\r\n\r\n";
   
    std::string buf = i.str();
    sock.write(buf.c_str(), buf.size());

    std::cerr<<"result\n";

    boost::cmt::asio::tcp::socket::iterator cur(&sock);
    boost::cmt::asio::tcp::socket::iterator end;
  //  std::string str;
    std::vector<char> c( cur,end );
    std::cerr<<"c.size:"<<c.size()<<std::endl;
    std::cerr<<std::string(c.begin(),c.end())<<std::endl;
    /*
    std::cerr<<str<<std::endl;
    while( cur != end ) {
        std::cerr<<*cur;
        std::cerr.flush();
        ++cur;
    }
    */
    std::cerr<<"done\n";
} catch ( const boost::exception& e ) {
    elog("%1%", boost::diagnostic_information(e) );
}
}

int main( int argc, char** argv ){
    boost::cmt::async(main2);
    boost::cmt::exec();
    return 0;
}
