#ifndef _BOOST_CMT_BASIC_STREAM_BUF_HPP_
#define _BOOST_CMT_BASIC_STREAM_BUF_HPP_
#include <boost/asio/streambuf.hpp>
#include <boost/cmt/asio.hpp>

namespace boost { namespace cmt { namespace asio {

template <typename SocketType, typename Allocator = std::allocator<char> >
class socket_streambuf : public boost::asio::basic_streambuf<Allocator> {
    public:
      /**
       * Constructs a streambuf with the specified maximum size. The initial size
       * of the streambuf's input sequence is 0.
       */
      explicit socket_streambuf( SocketType& sock,
          std::size_t max_size = (std::numeric_limits<std::size_t>::max)(),
          const Allocator& allocator = Allocator())
        : boost::asio::basic_streambuf<Allocator>(max_size, allocator),m_sock(sock)
      {
      }
    protected:
      /**
       * Behaves according to the specification of @c std::streambuf::underflow().
       */
      int underflow()
      {
        if (gptr() < pptr())
        {
          setg(&buffer_[0], gptr(), pptr());
          return traits_type::to_int_type(*gptr());
        }
        else
        {
           uint32_t av = m_sock.bytes_available();
           char tmp[1024];
           size_t s = boost::cmt::read_some( m_sock, boost::asio::buffer(tmp,sizeof(tmp)) );
           prepare(s) 
           memcpy(boost::asio::buffer_cast<char*>(prepare(tmp)), tmp, s);
           commit(s);
           
           if (gptr() < pptr()) {
              setg(&buffer_[0], gptr(), pptr());
              return traits_type::to_int_type(*gptr());
           }
            
           return traits_type::eof();
        }
      }

       SocketType& m_sock;
};


} } } // namespace boost::cmt::asio

#endif // _BOOST_CMT_BASIC_STREAM_BUF_HPP_
