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

#include <mace/rpc/json/read_value.hpp>

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
    os <<"{\"first\":";
    to_json( f(v.first), os, f );
    os <<",\"second\":";
    to_json( f(v.second), os, f );
    os<<'}';
  }
  template<typename V, typename Stream, typename Filter >
  void to_json( const std::pair<std::string,V>& v, Stream& os, Filter& f ) {
    os << '{';
    to_json( f(v.first),os,f);
    os<<':';
    to_json( f(v.second),os,f);
    os<<'}';
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
        to_json( base64_encode(&v.front(),v.size() ), os, f );
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

  /*
  template<typename T, typename Stream, typename F>
  void from_json( int64_t& i, Stream& in, F&)     {i=detail::parse_int(in); }
  template<typename T, typename Stream, typename F>
  void from_json( uint64_t& i, Stream& in, F&)    {i=detail::parse_uint(in);}
  template<typename T, typename Stream, typename F>
  void from_json( int32_t& i, Stream& in, F&)     {i=detail::parse_int(in); }
  template<typename T, typename Stream, typename F>
  void from_json( uint32_t& i, Stream& in, F&)    {i=detail::parse_uint(in);}
  template<typename T, typename Stream, typename F>
  void from_json( int16_t& i, Stream& in, F&)     {i=detail::parse_int(in); }
  template<typename T, typename Stream, typename F>
  void from_json( uint16_t& i, Stream& in, F&)    {i=detail::parse_uint(in);}
  template<typename T, typename Stream, typename F>
  void from_json( int8_t& i, Stream& in, F&)      {i=detail::parse_int(in); }
  template<typename T, typename Stream, typename F>
  void from_json( uint8_t& i, Stream& in, F&)     {i=detail::parse_uint(in);}
  template<typename T, typename Stream, typename F>
  void from_json( bool& i, Stream& in, F&) {
    char c; 
    string s;
    in.read(&c,1);
    while( c != ' ' && c !=','
    switch( c ) {
      case '0':

      case 't':

      case 'f':
    }
  }
  */






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
    static T unpack( Filter& f, const std::vector<char>& d ) {
      T tmp;
      /*
      datastream<const char*> ds(&d.front(),d.size());
      json::unpack( f, ds, tmp );
      */
      return tmp;
    }
  };

} } } // mace::rpc

#endif // MACE_RPC_RAW_RAW_IO_HPP
