#ifndef _MACE_RPC_RAW_RAW_IO_HPP_
#define _MACE_RPC_RAW_RAW_IO_HPP_
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

  template<typename T, typename Stream, typename Filter>
  void to_json( const T&, Stream& os, Filter& f );

  //! [Define base cases]
  template<typename T, typename Stream, typename Filter>
  void to_json( const std::vector<T>& v, Stream& os, Filter& f );

  template<typename T, typename Stream, typename Filter>
  void to_json( const std::set<T>& v, Stream& os, Filter& f );

  template<typename T, typename Stream, typename Filter>
  void to_json( const std::list<T>& v, Stream& os, Filter& f );

  template<typename K, typename V, typename Stream, typename Filter>
  void to_json( const std::map<K, V>& v, Stream& os, Filter& f );

  template<typename K, typename V, typename Stream, typename Filter>
  void to_json( const std::pair<K, V>& v, Stream& os, Filter& f );

  template<typename V, typename Stream, typename Filter>
  void to_json( const std::pair<std::string, V>& v, Stream& os, Filter& f );

  template< typename V, typename Stream, typename Filter>
  void to_json( const std::map<std::string, V>& v, Stream& os, Filter& f );

  template< typename Iterator, typename Stream, typename Filter>
  void to_json_array( Iterator b, Iterator e, Stream& os, Filter& f );

  template<typename T, typename Stream, typename Filter>
  void to_json( const std::vector<char>& v, Stream& os, Filter& f );

  template<typename Stream, typename Filter>
  void to_json( const std::string& s,    Stream& os, Filter& f ) { os << '"'<<escape_string(s)<<'"'; }
  template<typename Stream, typename Filter>
  void to_json( const uint64_t& i,       Stream& os, Filter& f ) { os << i;                       }
  template<typename Stream, typename Filter>
  void to_json( const int64_t& i,        Stream& os, Filter& f ) { os << i;                       }
  template<typename Stream, typename Filter>
  void to_json( const uint32_t& i,       Stream& os, Filter& f ) { os << i;                       }
  template<typename Stream, typename Filter>
  void to_json( const int32_t& i,        Stream& os, Filter& f ) { os << i;                       }
  template<typename Stream, typename Filter>
  void to_json( const uint16_t& i,       Stream& os, Filter& f ) { os << i;                       }
  template<typename Stream, typename Filter>
  void to_json( const int16_t& i,        Stream& os, Filter& f ) { os << i;                       }
  template<typename Stream, typename Filter>
  void to_json( const uint8_t& i,        Stream& os, Filter& f ) { os << i;                       }
  template<typename Stream, typename Filter>
  void to_json( const int8_t& i,         Stream& os, Filter& f ) { os << i;                       }
  template<typename Stream, typename Filter>
  void to_json( const double& d,         Stream& os, Filter& f ) { os << d;                       }
  template<typename Stream, typename Filter>
  void to_json( const float& v,          Stream& os, Filter& f ) { os << v;                       }
  template<typename Stream, typename Filter>
  void to_json( const bool& b,           Stream& os, Filter& f ) { os << (b ?  "true" : "false"); }
  template<typename Stream, typename Filter>
  void to_json( const signed_int& i,     Stream& os, Filter& f ) { os << i.value;                 }
  template<typename Stream, typename Filter>
  void to_json( const unsigned_int& i,   Stream& os, Filter& f ) { os << i.value;                 }
  //! [Define base cases]

  //! [Define a visitor]
  template<typename T,typename Stream, typename Filter>
  struct to_json_visitor {
      to_json_visitor( const T& v, Stream& _os, Filter& f ):val(v),os(_os),i(0),filt(f){}

      template<typename MemberPtr, MemberPtr m>
      void operator()( const char* name )const {
        if( i == 0 ) os << '{';    
        os<<'"'<<name<<"\":";
        to_json( filt(val.*m), os, filt);
        if( i != mace::reflect::reflector<T>::total_member_count-1 ) os << ',';
        if( i == mace::reflect::reflector<T>::total_member_count-1 ) os << '}';
        ++i;
      }
      const T& val;
      Stream& os;
      mutable int i;
      Filter& filt;
  };
  //! [Define a visitor]
  
  template<typename Iterator, typename Stream, typename Filter>
  void to_json_array( Iterator b, Iterator e, Stream& os, Filter& f ) {
    os << '[';
    while( b != e ) {
      to_json( f(*b), os, f );
      ++b;
      if( b != e ) { os <<','; }
    }
    os << ']';
  }
  template<typename K, typename V, typename Stream, typename Filter >
  void to_json( const std::pair<K,V>& v, Stream& os, Filter& f ) {
    os <<'[';
    to_json( f(v.first), os, f );
    os <<',';
    to_json( f(v.second), os, f );
    os<<']';
  }

  template<typename T, typename Stream, typename Filter >
  void to_json( const std::vector<T>& v, Stream& os, Filter& f ) {
    to_json_array( v.begin(), v.end(), os, f );
  }
  template<typename T, typename Stream, typename Filter >
  void to_json( const std::set<T>& v, Stream& os, Filter& f ) {
    to_json_array( v.begin(), v.end(), os, f );
  }
  template<typename T, typename Stream, typename Filter >
  void to_json( const std::list<T>& v, Stream& os, Filter& f ) {
    to_json_array( v.begin(), v.end(), os, f );
  }
  template<typename K, typename V, typename Stream, typename Filter >
  void to_json( const std::map<K,V>& v, Stream& os, Filter& f ) {
    to_json_array( v.begin(), v.end(), os, f );
  }

  template<typename T, typename Stream, typename Filter>
  void to_json( const std::vector<char>& v, Stream& os, Filter& f ) {
    if( v.size() )
        to_json( base64_encode((unsigned char*)&v.front(),v.size() ), os, f );
    else
        to_json( base64_encode(0,0), os, f );
  }

  template<typename Filter, typename Stream, int N>
  struct sequence_to_json {
     sequence_to_json( Filter& _f, Stream& _s ):i(0),f(_f),os(_s){}

     mutable int i;
     Filter&     f;
     Stream&     os;
     
     template<typename T>
     void operator() ( const T& v )const {
        to_json(v,os,f);
        ++i;
        if( i < N ) os << ',';
     }
  };

  /**
   *  Apply to any boost::fusion::sequence
   */
  template<typename Seq, typename Stream, typename Filter>
  void to_json_sequence( const Seq& v, Stream& os, Filter& f ) {
    sequence_to_json<Filter,Stream,boost::fusion::result_of::size<Seq>::type::value> pack_vector(f, os);
    os <<'[';
    boost::fusion::for_each( v, pack_vector );
    os <<']';
  }

  template<typename V, typename Stream, typename Filter >
  void to_json( const std::map<std::string,V>& v, Stream& os, Filter& f ) {
    os << '{';
    auto i = v.begin();
    while( i != v.end() ) {
      to_json( f(i->first), os, f ); 
      os<<':';
      to_json( f(i->second), os, f );
      ++i;
      if( i != v.end() ) os <<',';
    }
    os << '}';
  }

  template<typename IsReflected=boost::false_type>
  struct if_reflected {
    template<typename T,typename Filter, typename Stream>
    static inline void to_json( Filter& f,Stream& s, const T& v ) { 
      // Use boost serialization or die!
      elog( "Unknown type %1%", mace::reflect::get_typename<T>() );
    }
  };
  template<>
  struct if_reflected<boost::true_type> {
    template<typename T,typename Filter, typename Stream>
    static inline void to_json( const T& v, Stream& os, Filter& f ) {
      mace::reflect::reflector<T>::visit( to_json_visitor<T,Stream,Filter>( v, os, f ) );
    }
  };

  template<bool IsFusionSeq> struct if_fusion_seq {
    template<typename T,typename Stream, typename Filter> 
    inline static void to_json( const T& v, Stream& os, Filter& f ) {
        to_json_sequence( v, os, f );
    }
  };
  template<> struct if_fusion_seq<false> {
    template<typename T,typename Stream, typename Filter> 
    inline static void to_json( const T& v, Stream& os, Filter& f ) {
      if_reflected<typename mace::reflect::reflector<T>::is_defined>::to_json(v,os,f);
    }
  };
   
  template<typename T, typename Stream, typename Filter>
  void to_json( const T& v, Stream& os, Filter& filter ) {
    typedef typename std::remove_reference<decltype(filter(v))>::type filtered_type;
    if_fusion_seq< boost::fusion::traits::is_sequence<filtered_type>::value >::to_json(filter(v),os,filter);
  }


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
   *   If it starts with [" or { reads until matching ]" or }
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
     // read until closing ]} or " ignoring escaped "
     while( itr != end ) {
       if( !in_quote ) {
         switch( *itr ) {
           case '[':
           case '{': ++depth;         break;
           case ']':
           case '}': --depth;         break;
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
       ++itr;
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



  /**
   *  Converts a json string to rpc::value and reports errors
   */
  mace::rpc::value to_value( std::vector<char>&& d, error_collector& ec );
  mace::rpc::value to_value( char* start, char* end, error_collector& ec );

  struct json_io {
    template<typename T, typename Filter>
    static std::vector<char> pack( Filter& f, const T& v ) {
      std::stringstream ss;
      to_json( v, ss, f );
      std::string s = ss.str();
      std::vector<char> rv(s.size());
      if( s.size() )
        memcpy(  &rv.front(), s.c_str(), s.size());
      return rv;
    }

    template<typename T, typename Filter>
    static T unpack( Filter& f, std::vector<char>&& d ) {
      T tmp;
      json::error_collector ec;
      // convert json string into rpc::value
      mace::rpc::value v = to_value( std::move(d), ec );
      // convert rpc value into T applying filter f
      mace::rpc::unpack( f, v, tmp );
      return tmp;
    }
  };

} } } // mace::rpc::json

#endif // MACE_RPC_RAW_RAW_IO_HPP
