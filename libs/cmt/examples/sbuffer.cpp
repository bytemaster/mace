#include <boost/cmt/asio.hpp>
#include <boost/cmt/asio/streambuf.hpp>

void main2()
{
    boost::asio::io_service is;
    boost::asio::ip::tcp::socket sock(is);

    boost::cmt::socket_streambuf ssb(sock);
    boost::istream istr(&ssb);
    
    std::string s;
    istr >> s;
    std::cerr<<s;
}
int main( int argc, char** argv ) {
    boost::cmt::async(main2)  
    return boost::cmt::exec();
}
