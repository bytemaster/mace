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

namespace mace { namespace rpc { namespace http {

/// A request received from a client.
struct request {
  request()
  :keep_alive(true),http_version_major(1),http_version_minor(1){}

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
  std::string content;
};

} } } // mace::rpc::http

#endif // MACE_RPC_HTTP_REQUEST_HPP
