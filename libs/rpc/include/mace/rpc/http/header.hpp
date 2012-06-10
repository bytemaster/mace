//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MACE_RPC_HTTP_HEADER_HPP
#define MACE_RPC_HTTP_HEADER_HPP
#include <string>

namespace mace { namespace rpc { namespace http {

    struct header {
      header(){}

      header( header&& h )
      :name(std::move(h.name)),value(std::move(h.value)){}

      template<typename N, typename V>
      header( N&& n, V&& v )
      :name(std::forward<N>(n)),value( std::forward<V>(v)){}

      header& operator=( header&& h ) {
        std::swap(h.name,name);
        std::swap(h.value,value);
        return *this;
      }

      std::string name;
      std::string value;
    };

} } } // mace::rpc::http



#endif // MACE_RPC_HTTP_HEADER_HPP
