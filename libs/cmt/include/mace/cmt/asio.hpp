/**
 *  @file mace/cmt/asio.hpp
 *  @brief defines wrappers for boost::asio functions
 */
#ifndef _MACE_CMT_ASIO_HPP_
#define _MACE_CMT_ASIO_HPP_
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <mace/cmt/future.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/combine.hpp>

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

    /** 
     *  @brief wraps boost::asio::async_read
     *  @pre s.non_blocking() == true
     *  @return the number of bytes read.
     */
    template<typename AsyncReadStream, typename MutableBufferSequence>
    size_t read( AsyncReadStream& s, const MutableBufferSequence& buf ) {
        BOOST_ASSERT( s.non_blocking() );
        if( !s.non_blocking() ) { s.non_blocking(true); }
        boost::system::error_code ec;
        size_t r = boost::asio::read( s, buf, ec );
        if( ec ) {
            if( ec == boost::asio::error::would_block ) {
               promise<size_t>::ptr p(new promise<size_t>("mace::cmt::asio::read"));
               boost::asio::async_read( s, buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
               return p->wait();;
            }
            BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
        }
        return r;
    }
    /** 
     *  @brief wraps boost::asio::async_read_some
     *  @pre s.non_blocking() == true
     *  @return the number of bytes read.
     */
    template<typename AsyncReadStream, typename MutableBufferSequence>
    size_t read_some( AsyncReadStream& s, const MutableBufferSequence& buf ) {
        BOOST_ASSERT( s.non_blocking() );
     //   if( !s.non_blocking() ) { s.non_blocking(true); }
        boost::system::error_code ec;
        size_t r = s.read_some( buf, ec );
        if( ec ) {
            if( ec == boost::asio::error::would_block ) {
               promise<size_t>::ptr p(new promise<size_t>("mace::cmt::asio::read_some"));
               s.async_read_some( buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
               return p->wait();
            }
            BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
        }
       // promise<size_t>::ptr p(new promise<size_t>("mace::cmt::asio::read_some"));
       // s.async_read_some( buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return r;
    }

    /** @brief wraps boost::asio::async_write
     *  @return the number of bytes written
     */
    template<typename AsyncWriteStream, typename ConstBufferSequence>
    size_t write( AsyncWriteStream& s, const ConstBufferSequence& buf ) {
        BOOST_ASSERT( s.non_blocking() );
    //    if( !s.non_blocking() ) { s.non_blocking(true); }
        boost::system::error_code ec;
        size_t r = boost::asio::write( s, buf, ec );
        if( ec ) {
            if( ec == boost::asio::error::would_block ) {
                promise<size_t>::ptr p(new promise<size_t>("mace::cmt::asio::write"));
                boost::asio::async_write( s, buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
                return p->wait();
            }
            BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
        }
        return r;
    }

    /** 
     *  @pre s.non_blocking() == true
     *  @brief wraps boost::asio::async_write_some
     *  @return the number of bytes written
     */
    template<typename AsyncWriteStream, typename ConstBufferSequence>
    size_t write_some( AsyncWriteStream& s, const ConstBufferSequence& buf ) {
        BOOST_ASSERT( s.non_blocking() );
      //  if( !s.non_blocking() ) { s.non_blocking(true); }
        boost::system::error_code ec;
        size_t r = s.write_some( buf, ec );
        if( ec ) {
            if( ec == boost::asio::error::would_block ) {
                promise<size_t>::ptr p(new promise<size_t>("mace::cmt::asio::write_some"));
                s.async_write_some( buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
                return p->wait();
            }
            BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
        }
        return r;
    }

    template<typename AsyncWriteStream>
    class sink : public boost::iostreams::sink {
      public:
    //     struct category : boost::iostreams::sink::category {};
        typedef char      type;

        sink( AsyncWriteStream& p ):m_stream(p){}
    
        std::streamsize write( const char* s, std::streamsize n ) {
          return mace::cmt::asio::write( m_stream, boost::asio::const_buffers_1(s,n) );
        }
        void close() { m_stream.close(); }
    
      private:
         AsyncWriteStream&      m_stream;
    };

    template<typename AsyncReadStream>
    class source : public boost::iostreams::source {
      public:
        //     struct category : boost::iostreams::sink::category {};
        typedef char      type;

        source( AsyncReadStream& p ):m_stream(p){}
    
        std::streamsize read( char* s, std::streamsize n ) {
          return mace::cmt::asio::read_some( m_stream, boost::asio::buffer(s,n) );
        }
        void close() { m_stream.close(); }
    
      private:
        AsyncReadStream&      m_stream;
    };
    template<typename AsyncStream>
    class io_device {
      public:
        typedef boost::iostreams::bidirectional_device_tag category;
        typedef char                                     char_type;

        io_device( AsyncStream& p ):m_stream(p){}
    
        std::streamsize write( const char* s, std::streamsize n ) {
          return mace::cmt::asio::write( m_stream, boost::asio::const_buffers_1(s,static_cast<size_t>(n)) );
        }
        std::streamsize read( char* s, std::streamsize n ) {
          try {
            return mace::cmt::asio::read_some( m_stream, boost::asio::buffer(s,n) );
          } catch ( const boost::system::system_error& e ) {
            if( e.code() == boost::asio::error::eof )  
                return -1;
            throw;
          }
        }
        void close() { m_stream.close(); }
    
      private:
        AsyncStream&      m_stream;
    };


    namespace tcp {
        typedef boost::asio::ip::tcp::endpoint endpoint;
        typedef boost::asio::ip::tcp::resolver::iterator resolver_iterator;
        typedef boost::asio::ip::tcp::resolver resolver;
        std::vector<endpoint> resolve( const std::string& hostname, const std::string& port );

        /** @brief wraps boost::asio::async_accept
          * @post sock is connected
          * @post sock.non_blocking() == true  
          * @throw on error.
          */
        template<typename SocketType, typename AcceptorType>
        void accept( AcceptorType& acc, SocketType& sock ) {
            promise<boost::system::error_code>::ptr p( new promise<boost::system::error_code>("mace::cmt::asio::tcp::accept") );
            acc.async_accept( sock, boost::bind( mace::cmt::asio::detail::error_handler, p, _1 ) );
            auto ec = p->wait();
            if( !ec ) sock.non_blocking(true);
            if( ec ) BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
        }

        /** @brief wraps boost::asio::socket::async_connect
          * @post sock.non_blocking() == true  
          * @throw on error
          */
        template<typename AsyncSocket, typename EndpointType>
        void connect( AsyncSocket& sock, const EndpointType& ep ) {
            promise<boost::system::error_code>::ptr p(new promise<boost::system::error_code>("mace::cmt::asio::tcp::connect"));
            sock.async_connect( ep, boost::bind( mace::cmt::asio::detail::error_handler, p, _1 ) );
            auto ec = p->wait();
            if( !ec ) sock.non_blocking(true);
            if( ec ) BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
        }
      
        typedef boost::iostreams::stream<mace::cmt::asio::sink<boost::asio::ip::tcp::socket> >      ostream;
        typedef boost::iostreams::stream<mace::cmt::asio::source<boost::asio::ip::tcp::socket> >    istream;
        typedef boost::iostreams::stream<mace::cmt::asio::io_device<boost::asio::ip::tcp::socket> > iostream;

    }
    namespace udp {
        typedef boost::asio::ip::udp::endpoint endpoint;
        typedef boost::asio::ip::udp::resolver::iterator resolver_iterator;
        typedef boost::asio::ip::udp::resolver resolver;
        /// @brief resolve all udp::endpoints for hostname:port
        std::vector<endpoint> resolve( resolver& r, const std::string& hostname, const std::string& port );
    }


} } } // namespace mace::cmt::asio

#endif // _BOOST_CMT_ASIO_HPP_
