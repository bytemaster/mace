#include <mace/cmt/asio/udp/socket.hpp>
#include <mace/cmt/thread.hpp>
#include <boost/bind.hpp>

namespace mace { namespace cmt { namespace asio { namespace udp {

    void rw_handler( mace::cmt::promise<size_t>::ptr p, const boost::system::error_code& ec, size_t s )
    {
        if( !ec )
            p->set_value(s);
        else 
            p->set_exception( boost::copy_exception( boost::system::system_error(ec) ) );
    }

    size_t receive_from( boost::asio::ip::udp::socket& s, 
                                      char* data, size_t data_len, boost::asio::ip::udp::endpoint& ep ) {
        if( s.available()  ) {
            return s.receive_from( boost::asio::buffer(data,data_len), ep );
        }	
        promise<size_t>::ptr prom(new promise<size_t>());
        s.async_receive_from( boost::asio::buffer(data,data_len),  ep,
                                boost::bind( rw_handler, prom, 
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred ) );
        return prom->wait();
    }

    // send_to almost never blocks and the overhead associated with creating an async callback handler,
    // promise, etc slows down sending.
    size_t send_to( boost::asio::ip::udp::socket& s, 
                                 const char* data, size_t data_len, const boost::asio::ip::udp::endpoint& ep ) {
        return s.boost::asio::ip::udp::socket::send_to( boost::asio::buffer(data,data_len),  ep );
    }

} } } } // namespace mace::cmt::asio::udp
