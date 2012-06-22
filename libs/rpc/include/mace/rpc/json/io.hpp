#ifndef _MACE_RPC_JSON_JSON_IO_HPP_
#define _MACE_RPC_JSON_JSON_IO_HPP_
#include <mace/rpc/filter.hpp>
#include <mace/reflect/reflect.hpp>
#include <sstream>
#include <iostream>
#include <map>
#include <iomanip>
#include <sstream>

#include <boost/optional.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/include/size.hpp>

#include <mace/rpc/varint.hpp>
#include <mace/rpc/base64.hpp>

#include <mace/rpc/value.hpp>
#include <mace/rpc/value_io.hpp>
#include <mace/rpc/json/error_collector.hpp>

namespace mace { namespace rpc { namespace json { 

  /**
   *  Placeholder for unparsed json data.
   */
  class string {
    public:
      template<typename T>
      string( T&& v ):json_data( std::forward<T>(v) ){}
      string( const string& s ):json_data(s.json_data){}
      string( string& s ):json_data(s.json_data){}
      string( string&& s ):json_data(std::move(s.json_data)){}
      string(){}

      template<typename T>
      string& operator=( T&& t ) {
        json_data = std::forward<T>(t);
        return *this;
      }
      string& operator=( string&& s ) {
        json_data = std::move(s.json_data);
        return *this;
      }
      string& operator=( const string& s )
      {
        json_data = s.json_data;
        return *this;
      }

      
      
      std::string json_data;
  };

  /**
   *  @brief escapes non-printable or illegal characters.
   *
   *  Converts " to "\""
   *  Converts '\n' to "\n"
   *  Converts '\t' to "\t"
   *  Converts '\r' to "\r"
   *  Converts '\' to "\\"
   *  Converts unprintable characters to \x[HEX]
   *
   *  @note This method does not add the begining and ending quotes,
   *        it merely escapes the 'internal' part of the string.
   */
  std::string escape_string( const std::string s );

  /**
   *  @brief the inverse of escape_string()
   *  
   *   ASSERT( s == unescape_string(escape_string(s)) )
   */
  std::string unescape_string( const std::string s );

  /**
   *  Because escaped strings are always equal to or larger than
   *  the unescaped strings, you can reuse the same memory
   *  to unescape 'inplace' to avoid a copy.
   *
   *  @return a null terminated string starting at str.
   */
  char*       inplace_unescape_string( char* str );


  /**
   *   Returns the first non-whitespace char.
   */
  template<typename Iterator>
  bool skip_whitespace( Iterator& in, const Iterator& e ); 

  /**
   *   @tparam Iterator - an input iterator
   *
   *   Ignores leading white space.
   *   If it starts with [" or { reads until matching ]" or 
   *   If it starts with something else it reads until [{",]: or whitespace only
   *        allowing a starting - or a single .
   *
   *   Once you have a valid range, use 'from_json' to load the json into
   *   an object.
   *
   *   @note internal json syntax errors are not caught, only bracket errors 
   *         are caught by this method.  This makes it easy for error recovery
   *         when values are read sequentially.
   *
   *   @return a null-terminated vector<char> containing the first 'valid' json value
   */
  template<typename Iterator>
  std::vector<char> read_value( Iterator itr, const Iterator& end );  

  /**
   *  Converts a json string to rpc::value and reports errors
   */
  mace::rpc::value to_value( std::vector<char>&& d, error_collector& ec );
  mace::rpc::value to_value( char* start, char* end, error_collector& ec );
  std::map<std::string,json::string> read_key_vals( char* in, char* end, mace::rpc::json::error_collector& ec );

  struct io {
    template<typename T>
    static std::vector<char> pack( const T& v );

    template<typename T, typename Filter>
    static std::vector<char> pack( Filter& f, const T& v );
    
    template<typename T, typename Filter>
    static T unpack( Filter& f, std::vector<char>& d );

    template<typename T, typename Filter>
    static T unpack( Filter& f, std::vector<char>&& d );

    template<typename T>
    static T unpack( std::vector<char>&& d ) {
      function_filter<void> f;
      return unpack<T>( f, std::move(d) );
    }
  };
  template<typename T>
  std::string to_string( const T& v ) {
    auto i = io::pack(v);
    return std::string(&i.front(),i.size());
  }
  std::string pretty_print( std::vector<char>&& json, uint8_t indent = 2 );

  /**
   *  Similar to to_string, except that arrays and objects are
   *  nested and indented.
   */
  template<typename T>
  std::string to_pretty_string( const T& v, uint8_t indent = 2 ) {
    return pretty_print( io::pack(v), indent );
  }

} } } // mace::rpc::json

#include <mace/rpc/json/detail/io.ipp>

#endif // MACE_RPC_JSON_JSON_IO_HPP
