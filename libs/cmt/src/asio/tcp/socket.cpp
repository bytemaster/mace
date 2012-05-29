#include <mace/cmt/asio/tcp/socket.hpp>
#include <mace/cmt/asio.hpp>
#include <mace/cmt/thread.hpp>
#include <boost/array.hpp>


namespace mace { namespace cmt { namespace asio { namespace tcp {

    namespace detail {
        struct socket {
            socket( mace::cmt::asio::tcp::socket& s )
            :self(s),
             read_buf(0),cur_write_buf(NULL), cur_wbuf_idx(0), read_pos(0),last_avail(0)
            {}

            void  write_loop( uint8_t write_buf_idx );
            
            inline size_t read_some( char* buffer, size_t size );
            inline size_t read( char* buffer, size_t size );
            inline size_t write( const char* buffer, size_t size );

            mace::cmt::asio::tcp::socket& self;

            std::vector<char>   read_buf;
            std::vector<char>   write_buf[2];
            std::vector<char>*  cur_write_buf;

            uint8_t           cur_wbuf_idx;
            size_t            read_pos;
            size_t            last_avail;

            mace::cmt::future<void> write_complete;
        };
    }
  
    socket::socket()
    :boost::asio::ip::tcp::socket( mace::cmt::asio::default_io_service() ) {
        my = new detail::socket(*this);
    }
    
    socket::~socket() {
      try { flush(); } catch( ... ){}
      delete my;
      // By default this function always fails with operation_not_supported when used on Windows XP, Windows Server 2003
      #ifndef WIN32 
        try { cancel(); }
        catch( ... ) {}
      #endif
    }


    boost::system::error_code socket::connect( const boost::asio::ip::tcp::endpoint& ep ) {
        return mace::cmt::asio::tcp::connect( *this, ep );
    }

    inline size_t detail::socket::read_some( char* buf, size_t size ) {
  
        // if there is any data in the read buf, grab it first
        if( read_buf.size() && (read_buf.size() - read_pos) ) {
            int s = std::min( (read_buf.size()-read_pos), size );
            memcpy( buf, &read_buf[read_pos], s );
            size -= s;
            buf  += s;

            uint32_t new_rbs = read_buf.size() - s;

            if( new_rbs )
                read_pos = s;
            else
                read_buf.resize( 0 );

            if( size == 0 ) 
                return s;
        }
        // by the time I get here read_buf should be empty
        
        // if data is available, we should be able to read it without blocking
        if( (last_avail = self.available()) >= size ) {
            read_buf.resize( last_avail - size );
            boost::array<boost::asio::mutable_buffer, 2> bufs = {{  
                boost::asio::buffer( buf, size ),
                boost::asio::buffer( read_buf )
            }};
            int r =  boost::asio::read( (boost::asio::ip::tcp::socket&)self, bufs );
            last_avail -= r;
            return r;
        }

        // perform an async operation to read the rest 
        return mace::cmt::asio::read_some( self, boost::asio::buffer(buf,size) );
    }
    size_t socket::read_some( char* buf, size_t size ) {
        return my->read_some(buf,size);
    }
    inline size_t detail::socket::read( char* buf, size_t size ) {
        size_t r = 0;
        // if there is any data in the read buf, grab it first
        if( (read_buf.size() - read_pos) ) {
            int s = std::min( (read_buf.size()-read_pos), size );
            memcpy( buf, &read_buf[read_pos], s );

            size     -= s;
            buf      += s;
            read_pos += s;

            // if there is any data left over, move it forward
            if( read_pos == read_buf.size() ) {
                read_buf.resize( 0 );
                read_pos = 0;
            }

            if( size == 0 ) { return s; }
            r += s;
        }
        
        // by the time I get here read_buf should be empty
        if( (last_avail = self.available()) >= size ) {
            read_buf.resize( last_avail - size );
            boost::array<boost::asio::mutable_buffer, 2> bufs = {{  
                boost::asio::buffer( buf, size ),
                boost::asio::buffer( read_buf )
            }};
            boost::asio::read( (boost::asio::ip::tcp::socket&)self, bufs );
            return r + size;
                
        }
        return r + mace::cmt::asio::read( self, boost::asio::buffer(buf,size) );
    }
    size_t socket::read( char* buf, size_t size ) {
        return my->read(buf,size);
    }

    socket::iterator socket::iterator::operator++(int) {
        iterator tmp = *this;
        ++*this;
        return tmp;
    }
    socket::iterator& socket::iterator::operator++() {
        if( 1 != s->read(&value,1) ) {
            s = NULL;
        }
        return *this;
    }

    /**
     *  This method will loop until both write_buf's are empty.
     *
     */
    void detail::socket::write_loop( uint8_t write_buf_idx ) {
      do {
        size_t r = 0;
        size_t total_wrote = 0;
        const char* buffer = &write_buf[write_buf_idx].front();
        size_t      size   = write_buf[write_buf_idx].size();

        do {
            size_t wrote = mace::cmt::asio::write(self,boost::asio::buffer(buffer, size - total_wrote) );
            r += wrote;
            buffer += wrote;
            total_wrote += wrote;
        }while( r < size );
        write_buf[write_buf_idx].resize(0);

        if( cur_write_buf->size() ) {
          cur_wbuf_idx = (cur_wbuf_idx+1)&0x01;
          write_buf_idx = (write_buf_idx+1)&0x01;
          cur_write_buf = &write_buf[cur_wbuf_idx];
        } else {
          cur_wbuf_idx  = 0;
          cur_write_buf = NULL;
        }
      }while( cur_write_buf );
    }


    inline size_t detail::socket::write( const char* buffer, size_t size ) {
      if( write_complete.error() ) 
          write_complete.wait();
      if( size == 0 ) 
        return 0;

      bool first = false;
      if( cur_write_buf == NULL ) {
        first = true;
        cur_wbuf_idx  = 0;
        cur_write_buf = &write_buf[cur_wbuf_idx];
      }

      size_t wpos = cur_write_buf->size();
      cur_write_buf->resize( wpos + size );
      memcpy( &(*cur_write_buf)[wpos], buffer, size );

      if( first ) {
        write_complete = mace::cmt::async<void>( boost::bind( &socket::write_loop, this, cur_wbuf_idx ) );
        cur_wbuf_idx = (cur_wbuf_idx+1)&0x01;
        cur_write_buf = &write_buf[cur_wbuf_idx];
      }

      return size;
    }

    /**
     *  Alternate between two buffers, fill one while asio is writing the other,
     *  then switch.
     */
    size_t socket::write( const char* buffer, size_t size ) {
      return my->write(buffer,size);
    }

    void socket::flush() {
      if( my->write_complete.valid() )
        my->write_complete.wait();
    }

} } } }  // namespace mace::cmt::asio::tcp
