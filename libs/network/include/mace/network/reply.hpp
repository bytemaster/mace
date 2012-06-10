#ifndef _MACE_NETWORK_REPLY_HPP_
#define _MACE_NETWORK_REPLY_HPP_
namespace mace { namespace network {

namespace detail {
  class reply;
}

class reply : boost::noncopyable {
  public:
    /**
     *  If you want to monitor the status of reading the content, then
     *  specify a progress callback.  Return false to cancel the operation.
     *
     *  first param is bytes read/written, second param is the total bytes or
     *  0 if unknown (chunked)
     */
    typedef boost::function<bool(size_t,size_t)> progress_callback;

    enum status_type {
      ok = 200,
      created = 201,
      accepted = 202,
      no_content = 204,
      multiple_choices = 300,
      moved_permanently = 301,
      moved_temporarily = 302,
      not_modified = 304,
      bad_request = 400,
      unauthorized = 401,
      forbidden = 403,
      not_found = 404,
      internal_server_error = 500,
      not_implemented = 501,
      bad_gateway = 502,
      service_unavailable = 503
    };
    reply( status_type t );
    reply( reply&& );
    ~reply();

    reply& operator=(reply&&);

    reply&             status( status_type s );
    reply&             content( istream&& read_from );
    reply&             content( status_type s ); // use stock content for status type.
    reply&             header( std::string&& name, std::string&& value );
    reply&             keep_alive( bool b );
    bool               keep_alive()const;
                       
    bool               chunked()const;
    size_t             content_length()const;

    // short cut for   vector<char> v; reply.write_content_to(v);
    std::vector<char>  take_content();

    // can only be performed once
    void               write_content_to( ostream&& o, progress_callback&& cb = progress_callback() );

    const header_map&  headers()const;
    header_map&        headers();

    status_type status()const;

    void write( ostream&& o );
    void read( istream&& o );

  private:
    detail::reply* my;
};

} }

#endif // _MACE_NETWORK_REPLY_HPP_
