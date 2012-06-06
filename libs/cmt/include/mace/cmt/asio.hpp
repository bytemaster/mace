/**
 *  @file mace/cmt/asio.hpp
 *  @brief defines wrappers for boost::asio functions
 */
#ifndef _MACE_CMT_ASIO_HPP_
#define _MACE_CMT_ASIO_HPP_
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <mace/cmt/future.hpp>

namespace mace { namespace cmt { 
/**
 *  @brief defines mace::cmt wrappers for boost::asio functions.
 */
namespace asio {
    /**
     *  @brief internal implementation types/methods for mace::cmt::asio
     */
    namespace detail {
        using namespace mace::cmt;

        void read_write_handler( const promise<size_t>::ptr& p, 
                                 const boost::system::error_code& ec, 
                                size_t bytes_transferred );
        void read_write_handler_ec( promise<size_t>* p, 
                                    boost::system::error_code* oec, 
                                    const boost::system::error_code& ec, 
                                    size_t bytes_transferred );
        void error_handler( const promise<boost::system::error_code>::ptr& p, 
                              const boost::system::error_code& ec );
        void error_handler_ec( promise<boost::system::error_code>* p, 
                              const boost::system::error_code& ec ); 
    }
    /**
     * @return the default boost::asio::io_service for use with mace::cmt::asio
     * 
     * This IO service is automatically running in its own thread to service asynchronous
     * requests without blocking any other threads.
     */
    boost::asio::io_service& default_io_service();

    /** @brief wraps boost::asio::async_read
     *  @return the number of bytes read.
     */
    template<typename AsyncReadStream, typename MutableBufferSequence>
    cmt::future<size_t> read( AsyncReadStream& s, const MutableBufferSequence& buf ) {
        promise<size_t>::ptr p(new promise<size_t>());
        boost::asio::async_read( s, buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p;
    }
    /** @brief wraps boost::asio::async_read_some
     *  @return the number of bytes read.
     */
    template<typename AsyncReadStream, typename MutableBufferSequence>
    cmt::future<size_t> read_some( AsyncReadStream& s, const MutableBufferSequence& buf ) {
        promise<size_t>::ptr p(new promise<size_t>());
        s.async_read_some( buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p;
    }

    /** @brief wraps boost::asio::async_write
     *  @return the number of bytes written
     */
    template<typename AsyncReadStream, typename MutableBufferSequence>
    cmt::future<size_t> write( AsyncReadStream& s, const MutableBufferSequence& buf ) {
        promise<size_t>::ptr p(new promise<size_t>());
        boost::asio::async_write( s, buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p;
    }

    /** @brief wraps boost::asio::async_write_some
     *  @return the number of bytes written
     */
    template<typename AsyncReadStream, typename MutableBufferSequence>
    cmt::future<size_t> write_some( AsyncReadStream& s, const MutableBufferSequence& buf ) {
        promise<size_t>::ptr p(new promise<size_t>());
        s.async_write_some(  buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p;
    }

    namespace tcp {
        typedef boost::asio::ip::tcp::endpoint endpoint;
        typedef boost::asio::ip::tcp::resolver::iterator resolver_iterator;
        typedef boost::asio::ip::tcp::resolver resolver;
        /// @brief asynchronously resolve all tcp::endpoints for hostname:port
        cmt::future<std::vector<endpoint> > resolve( const std::string& hostname, const std::string& port );

        /// @brief wraps boost::asio::async_accept
        template<typename SocketType, typename AcceptorType>
        cmt::future<boost::system::error_code> accept( AcceptorType& acc, SocketType& sock ) {
            promise<boost::system::error_code>::ptr p( new promise<boost::system::error_code>() );
            acc.async_accept( sock, boost::bind( mace::cmt::asio::detail::error_handler, p, _1 ) );
            return p;
        }

        /// @brief wraps boost::asio::socket::async_connect
        template<typename AsyncSocket, typename EndpointType>
        cmt::future<boost::system::error_code> connect( AsyncSocket& sock, const EndpointType& ep ) {
            promise<boost::system::error_code>::ptr p(new promise<boost::system::error_code>());
            sock.async_connect( ep, boost::bind( mace::cmt::asio::detail::error_handler, p, _1 ) );
            return p;
        }
    }
    namespace udp {
        typedef boost::asio::ip::udp::endpoint endpoint;
        typedef boost::asio::ip::udp::resolver::iterator resolver_iterator;
        typedef boost::asio::ip::udp::resolver resolver;
        /// @brief asynchronously resolve all udp::endpoints for hostname:port
        cmt::future<std::vector<endpoint> > resolve( resolver& r, const std::string& hostname, const std::string& port );
    }


} } } // namespace mace::cmt::asio

#endif // _BOOST_CMT_ASIO_HPP_
