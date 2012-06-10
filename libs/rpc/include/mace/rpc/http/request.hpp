//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MACE_RPC_HTTP_REQUEST_HPP
#define MACE_RPC_HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include <mace/rpc/http/header.hpp>
#include <utility>
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>

namespace mace { namespace rpc { namespace http {

class content : boost::noncopyable {
  public:
      template<typename String>
      content( String s ):m_value( std::forward<String>(s) ){}
      content();

      content( content&& c ):m_value(std::move(c.m_value)){}

      size_t size()const { return m_value.size(); }
      const std::string& value()const { return m_value; }
      operator std::string()const { return m_value; }

      std::string take() { return std::move(m_value); }

  private:
      std::string m_value;
};

/// A request received from a client.
struct request : boost::noncopyable{
  request()
  :keep_alive(true),http_version_major(1),http_version_minor(1){}

  request( request&& m )
  :method(std::move(m.method)),
   keep_alive(m.keep_alive),
   uri( std::move(m.uri) ),
   http_version_major(m.http_version_major),
   http_version_minor(m.http_version_minor),
   headers( std::move(m.headers) ),
   content( std::move(m.content) ) {}

  /// The request method, e.g. "GET", "POST".
  std::string method;

  bool keep_alive;

  /// The requested URI, such as a path to a file.
  std::string uri;

  /// Major version number, usually 1.
  int http_version_major;

  /// Minor version number, usually 0 or 1.
  int http_version_minor;

  /// The headers included with the request.
  std::vector<header> headers;

  /// The optional content sent with the request.
  std::string   content;

  friend request& operator << ( request& r, const header& h ) {
    r.headers.push_back(h);
    return r;
  }
  friend request& operator << ( request& r, header&& h ) {
    r.headers.push_back(std::move(h));
    return r;
  }
  friend request operator << ( request&& r, header&& h ) {
    r.headers.push_back(std::move(h));
    return std::move(r);
  }
  friend request& operator << ( request& r, http::content&& h ) {
    r.content += h.take();
    return r;
  }
  friend request operator << ( request&& r, http::content&& h ) {
    r.content += h.take();
    return std::move(r);
  }
  

  template<typename Stream>
  friend Stream& operator << (Stream& s, const request& r ) {
    std::string request_line = r.method + "  " + r.uri + " HTTP/";
    request_line += char('0' + r.http_version_major);
    request_line += '.';
    request_line += char('0' + r.http_version_minor);
    request_line += "\r\n";

    s.write(request_line.c_str(),request_line.size() );
    for( uint32_t i = 0; i < r.headers.size(); ++i ) {
      if( r.headers[i].name != "content-length" ){
        s.write(r.headers[i].name.c_str(),r.headers[i].name.size());
        s.write(": ",2);
        s.write(r.headers[i].value.c_str(),r.headers[i].value.size());
        s.write("\r\n",2);
      }
    }
    std::string cl;
    if( r.content.size() )
      cl = "content-length: " + boost::lexical_cast<std::string>(r.content.size()) + "\r\n";
    cl += "\r\n";
    s.write(cl.c_str(),cl.size());
    s.write(r.content.c_str(), r.content.size() );
    return s;
  }
};

inline request get( const std::string& uri ) {
  request r;
  r.method = "GET";
  r.uri = uri;
  return r;
}

} } } // mace::rpc::http

#endif // MACE_RPC_HTTP_REQUEST_HPP
