#ifndef _MACE_ASIO_ACCEPTOR_HPP
#define _MACE_ASIO_ACCEPTOR_HPP
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>

namespace mace { namespace cmt {  namespace asio {
    typedef boost::asio::ip::tcp::socket  tcp_socket;
    typedef boost::shared_ptr<tcp_socket> tcp_socket_ptr;

    /** 
     *  @class acceptor
     *  @brief Accepts new tcp sockets.
     *
     *  Provides a means to async listen for new tcp connections.  The specified callback
     *  handler will be called any time a new connection arrives.  
     *
     *  Unlike the boost::asio::tcp::acceptor, on_new_connection will be called in the current thread as
     *  a new 'fiber'.
     *
     *  @code
        acceptor acc;
        acc.listen( 8000, on_new_connection );
     *  @endcode
     */
    class acceptor : public boost::enable_shared_from_this<acceptor> {
        public:
            typedef boost::shared_ptr<acceptor> ptr;
            typedef boost::function<void(const tcp_socket_ptr,boost::system::error_code)> handler;

            acceptor();
            ~acceptor();

            void listen( uint16_t port, const handler& on_con );

        private:
            class acceptor_private* my;
    };

} } } // mace::cmt::asio

#endif // _MACE_ASIO_ACCEPTOR_HPP
