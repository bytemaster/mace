#include <mace/network/detail/stream.hpp>
namespace mace { namespace network {

/**
 *  In order to support asynchronous operations that allow
 *  requests to 'stream' to the socket from a file or
 *  a buffer, this class abstracts the data access method
 *  allowing the network code to optimize reads and writes.
 *  
 */
class ostream : boost::noncopyable {
  public:
    ostream( ostream&& o );
    ~ostream();

    // will write to d and resize if necessary
    ostream( std::vector<char>& d );
    ostream( std::string& d );

    // write write up to l bytes to d
    ostream( char* d, size_t l );

    // wrap an std::ostream
    ostream( std::ostream& os );

    template<typename Stream>
    ostream( Stream&& s )
    :my( new detail::ostream( std::forward<Stream>(s) ) ){}

    // attempt to write s bytes, return number actually written
    size_t write( char*, size_t s );
    
    // true when there is no more space... 
    bool   eof();

    // if random access is available
    size_t size()const;
    char*  buffer()const;

  private:
    detail::ostream* my;
};

/**
 *
 */
class istream : boost::noncopyable {
  public:
    istream( istream&& c );
    ~istream();
    
    // read from data
    istream( const std::string& data );
    istream( const std::vector<char>& data );
    istream( std::vector<char>&& data );
    istream( const char* data, size_t l );

    // if max set, it will limit the amount that may
    // be read from the stream
    istream( std::istream& i, size_t max = -1 );

    template<typename IStream>
    istream( IStream&& s )
    :my( new detail::istream( std::forward<IStream>(s) ) ){}

    // attempt to write c bytes, return amount read.
    size_t read( char* c, size_t b );

    // true when there is no more data
    bool   eof();

    // if random access is available, else 0
    size_t      size()const;
    const char* buffer()const;
  private:
    detail::ostream* my;
};

} }
