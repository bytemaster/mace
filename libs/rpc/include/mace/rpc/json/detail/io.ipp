#ifndef _MACE_RPC_JSON_DETAIL_IO_IPP
#define _MACE_RPC_JSON_DETAIL_IO_IPP
#include <mace/rpc/json/io.hpp>
#include <mace/rpc/json/detail/to_json.hpp>

namespace mace { namespace rpc { namespace json {

  /**
   *   Returns the first non-whitespace char.
   */
  template<typename Iterator>
  bool skip_whitespace( Iterator& in, const Iterator& e ) {
     if( in == e ) 
       return false;
     char c = *in;
     // ignore leading whitespace
     while( c == ' ' || c == '\t' || c == '\n' || c == '\r' ) {
       ++in;
       if( in == e ) 
         return false;
       c = *in;
     }
     return true;
  }

  /**
   *   @tparam Iterator - an input iterator
   *
   *   Ignores leading white space.
   *   If it starts with [" or  reads until matching ]" or 
   *   If it starts with something else it reads until [{",}]: or whitespace only
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
  std::vector<char> read_value( Iterator itr, const Iterator& end ) {
     std::vector<char> buf;
     if( !skip_whitespace(itr,end) ) return buf;

     bool found_dot = false;
     // check for literal vs object, array or string
     switch( *itr ) {
//       std::cerr<<*itr;
       case '[':
       case '{':
       case '"':
         break;
       default: {  // literal
         // read until non-literal character
         // allow it to start with - 
         // allow only one '.' 
         while( itr != end ) {
           switch( *itr ) {
             case '[': 
             case '{': 
             case '}': 
             case ']':
             case '"': 
             case ',': 
             case ':': 
             case ' ': 
             case '\t': 
             case '\n': 
             case '\r': 
               return buf;
             case '.':
               if( found_dot ) 
                  return buf;
               found_dot = true;
               break;
             case '-':
               if( buf.size() ) { buf.push_back('\0'); return buf; }
           }
           buf.push_back(*itr);
           ++itr;
         }
         buf.push_back('\0'); 
         return buf; 
       }
     } // end literal check

     int depth = 0;
     bool in_quote = false;
     bool in_escape = false;
     bool done = 0;
     // read until closing ] or " ignoring escaped "
     while( itr != end && !done) {
//       std::cerr<<*itr;
       if( !in_quote ) {
         switch( *itr ) {
           case '[':
           case '{': ++depth;         break;
           case ']':
           case '}': --depth; done = (depth<=0);   break;
           case '"': 
             ++depth;
             in_quote = true; 
             break;
           default: // do nothing;
             break;
         }
       } else { // in quote
         switch( *itr ) {
           case '"': if( !in_escape ) {
             --depth;
             in_quote = false;
             break;
           }
           case '\\': 
             in_escape = !in_escape;
             break;
           default:
             in_escape = false;
         }
       }
       buf.push_back(*itr);
       if( !done ) ++itr;

       if( !depth )  {
         buf.push_back('\0'); 
         return buf; 
       }
    }
    if( depth != 0 ) {
     // TODO: Throw Parse Error!
     std::cerr<<"Parse Error!\n";
    }
    buf.push_back('\0'); 
    return buf;
  }

  template<typename T>
  std::vector<char> io::pack( const T& v ) {
      default_filter f;
      std::stringstream ss;
      detail::to_json( v, ss, f );
      std::string s = ss.str();
      std::vector<char> rv(s.size());
      if( s.size() )
        memcpy(  &rv.front(), s.c_str(), s.size());
      return rv;
 }

 template<typename T, typename Filter>
 std::vector<char> io::pack( Filter& f, const T& v ) {
   std::stringstream ss;
   detail::to_json( v, ss, f );
   std::string s = ss.str();
   std::vector<char> rv(s.size());
   if( s.size() )
     memcpy(  &rv.front(), s.c_str(), s.size());
   return rv;
 }

 template<typename T, typename Filter>
 T io::unpack( Filter& f, std::vector<char>& d ) {
   T tmp;
   json::error_collector ec;
   // convert json string into rpc::value
   mace::rpc::value v = to_value( std::move(d), ec );
   // convert rpc value into T applying filter f
   mace::rpc::unpack( f, v, tmp );
   return tmp;
 }

 template<typename T, typename Filter>
 T io::unpack( Filter& f, std::vector<char>&& d ) {
   T tmp;
   json::error_collector ec;
   // convert json string into rpc::value
   mace::rpc::value v = to_value( std::move(d), ec );
   // convert rpc value into T applying filter f
   mace::rpc::unpack( f, v, tmp );
   return tmp;
 }

} } }

#endif 
