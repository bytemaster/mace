#ifndef _MACE_RPC_JSON_DETAIL_TO_JSON_HPP_
#define _MACE_RPC_JSON_DETAIL_TO_JSON_HPP_
#include <mace/reflect/reflect.hpp>
#include <mace/rpc/value.hpp>

namespace mace { namespace rpc { namespace json { 

  namespace detail {

  template<typename T, typename Stream, typename Filter>
  void to_json( const T&, Stream& os, Filter& f );

  //! [Define base cases]
  template<typename T, typename Stream, typename Filter>
  void to_json( const mace::rpc::value&, Stream& os, Filter& f );

  template<typename Stream, typename Filter>
  void to_json( const void_t&, Stream& os, Filter& f ) { os<<"null"; }

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
  template<typename Stream, typename Filter>
  void to_json( const json::string& s,   Stream& os, Filter& f ) { os << s.json_data;             }
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
    static inline void to_json( const T& v, Stream& os, Filter& f ) { 
      // Use boost serialization or die!
      elog( "Unknown type %1%", mace::reflect::get_typename<T>() );
      assert( !"This should never be called" );
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
  template<> struct if_fusion_seq<0> {
    template<typename T,typename Stream, typename Filter> 
    inline static void to_json( const T& v, Stream& os, Filter& f ) {
      if_reflected<typename mace::reflect::reflector<T>::is_defined>::to_json(v,os,f);
    }
  };
   
  template<typename T, typename Stream, typename Filter>
  void to_json( const T& v, Stream& os, Filter& filter ) {
    typedef typename std::remove_reference<decltype(filter(v))>::type filtered_type;
    if_fusion_seq<boost::fusion::traits::is_sequence<filtered_type>::value>::to_json(filter(v),os,filter);
  }

  template<typename Stream>
  struct value_visitor : const_visitor {
    value_visitor( Stream& s ):os(s){}
    Stream& os;
    virtual void operator()( const int8_t& v      ){ os << v; }
    virtual void operator()( const int16_t& v     ){ os << v; }
    virtual void operator()( const int32_t& v     ){ os << v; }
    virtual void operator()( const int64_t& v     ){ os << v; }
    virtual void operator()( const uint8_t& v     ){ os << v; }
    virtual void operator()( const uint16_t& v    ){ os << v; }
    virtual void operator()( const uint32_t& v    ){ os << v; }
    virtual void operator()( const uint64_t& v    ){ os << v; }
    virtual void operator()( const float& v       ){ os << v; }
    virtual void operator()( const double& v      ){ os << v; }
    virtual void operator()( const bool& v        ){ os << v ? "true" : "false"; }
    virtual void operator()( const std::string& v ){ os << '"' << escape_string(v) <<'"'; }
    virtual void operator()( const object& o ){
      os << '{';
        for( uint32_t i = 0; i < o.fields.size(); ++i ) {
          if( i ) os <<',';
          (*this)( o.fields[i].key );
          os<<':';
          o.fields[i].val.visit( value_visitor(*this) );
        }
      os << '}';
    }
    virtual void operator()( const array& o ){
      os << '[';
        for( uint32_t i = 0; i < o.fields.size(); ++i ) {
          if( i ) os <<',';
          o.fields[i].visit( value_visitor(*this) );
        }
      os << ']';
    }
    virtual void operator()( ){ os << "null"; }
  };

  template<typename Stream, typename Filter>
  void to_json( const mace::rpc::value& v, Stream& os, Filter& f ) {
    v.visit( value_visitor<Stream>(os) );
  }

} // detail 
} } } // mace::rpc::json

#endif
