#ifndef _MACE_CMT_ASIO_TCP_SOCKET_HPP
#define _MACE_CMT_ASIO_TCP_SOCKET_HPP
#include <boost/asio.hpp>

namespace mace { namespace cmt { namespace asio { namespace tcp {
    namespace detail { struct socket; }

    /**
     *  Provides a buffered socket based on boost::asio.  
     *
     *  Read buffer grabs what ever is available from ASIO when it
     *  is empty.  This makes small synchronous reads much more effecient.
     */
    class socket  : public boost::asio::ip::tcp::socket {
        public:
            typedef boost::shared_ptr<socket> ptr;

            socket();
            ~socket();

            boost::system::error_code connect( const boost::asio::ip::tcp::endpoint& ep );

            /**
             * Reads one element at a time.
             */
            struct iterator : public std::iterator<std::input_iterator_tag,char,void> {
                iterator( mace::cmt::asio::tcp::socket* _s = NULL)
                :s(_s){ if(_s){++*this;}  }

                inline const char& operator*()const  { return value;  }
                inline const char* operator->()const { return &value; }
                inline char& operator*() { return value;  }
                inline char* operator->(){ return &value; }

                iterator& operator++();
                iterator operator++(int);

                bool operator == ( const iterator& i )const { return s == i.s; }
                bool operator != ( const iterator& i )const { return s != i.s; }

                private:
                    char                           value;
                    mace::cmt::asio::tcp::socket* s;
            };

            size_t read_some( char* buffer, size_t size );
            size_t read( char* buffer, size_t size );
            size_t write( const char* buffer, size_t size );

            void flush();
        private:
            detail::socket* my;
    };

} } } } // namespace mace::cmt::asio::tcp

#endif
