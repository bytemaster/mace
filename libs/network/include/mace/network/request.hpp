#ifndef _MACE_NETWORK_REQUEST_HPP_
#define _MACE_NETWORK_REQUEST_HPP_
#include <mace/network/url.hpp>

namespace mace { namespace network {

  namespace detail {
    class request;
  }

  typedef std::map<std::string,std::string> header_map;

  class request : boost::noncopyable {
    public:
      typedef boost::function<bool(size_t,size_t)> progress_callback;
      enum method_type {
        GET, POST, PUT, HEAD
      };

      request( url&& u = url() );
      request( request&& r );
      ~request();

      request& operator= ( request&& r );

      request&           url( network::url&& u );
      request&           method( request::method_type t );
      request&           header( std::string&& name, std::string&& value );
      request&           keep_alive( bool b );
      request&           content( istream&& read_from );

      bool               chunked()const;
      bool               keep_alive()const;
      const url&         url()const;
      const header_map&  headers()const;
      method_type        method()const;
      size_t             content_length()const;

      // grabs the content from the request, leaving the request empty.
      // this can only be done once because the input could be a stream
      // and the alternative would be to store the content.
      // Don't do this if your content is a 2GB file! 
      std::vector<char>  take_content();
      void               write_content_to( ostream&& o, progress_callback&& cb = progress_callback() );

      void write( ostream&& o );
      void read( istream&& o );
    private:
      detail::request* my;
  };

  request get( url&& u );
  request post( url&& u );
  request put( url&& u );
  request head( url&& u );

} }
#endif // _MACE_NETWORK_REQUEST_HPP_
