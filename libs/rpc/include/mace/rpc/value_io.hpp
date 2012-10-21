/**
 *  @file mace/rpc/value_io.hpp
 *
 *  @brief Packs and unpacks types to/from mace::rpc::value.
 *
 */
#ifndef _MACE_RPC_VALUE_IO_HPP_
#define _MACE_RPC_VALUE_IO_HPP_
#include <mace/reflect/reflect.hpp>
#include <mace/rpc/base64.hpp>
#include <mace/void.hpp>
#include <sstream>
#include <iostream>
#include <map>
#include <iomanip>

#include <mace/cmt/log/log.hpp>

#include <boost/optional.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <mace/rpc/value.hpp>
#include <mace/rpc/filter.hpp>

namespace mace { namespace rpc { 

  template<typename T, typename Filter> 
  void pack( Filter&, mace::rpc::value& jsv, const T& v ); 

  template<typename T, typename Filter> 
  void unpack( Filter&, const mace::rpc::value& jsv, T& v ); 

  template<typename T, typename Filter> 
  void pack( Filter&, mace::rpc::value& jsv, const boost::optional<T>& v );

  template<typename T, typename Filter> 
  void unpack( Filter&, const mace::rpc::value& jsv, boost::optional<T>& v );

  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const mace::rpc::value& v )  { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, mace::rpc::value& v )   { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, mace::rpc::value&& v )  { jsv = std::move(v); }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const void_t& v )       { jsv = mace::rpc::value(); }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const bool& v )         { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const float& v )        { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const double& v )       { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const uint8_t& v )      { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const uint16_t& v )     { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const uint32_t& v )     { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const uint64_t& v )     { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const int8_t& v )       { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const int16_t& v )      { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const int32_t& v )      { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const int64_t& v )      { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const std::string& v )  { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, std::string& v )        { jsv = v; }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, std::string&& v )       { jsv = std::move(v); }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const char* v )         { jsv = std::string(v); }

  template<typename Filter>
  void pack( Filter& c, mace::rpc::value& jsv, const std::vector<char>& value );
  template<typename T, typename Filter>
  void pack( Filter& c, mace::rpc::value& jsv, const std::vector<T>& value );
  template<typename T, typename Filter>
  void pack( Filter& c, mace::rpc::value& jsv, const std::list<T>& value );

  template<typename T, typename Filter>
  void pack( Filter& c, mace::rpc::value& jsv, const std::set<T>& value );
  template<typename Key, typename Value, typename Filter>
  void pack( Filter& c, mace::rpc::value& jsv, const std::map<Key,Value>& value );
  template<typename Key, typename Value, typename Filter>
  void pack( Filter& c, mace::rpc::value& jsv, const std::pair<Key,Value>& value );
  template<typename Value, typename Filter>
  void pack( Filter& c, mace::rpc::value& jsv, const std::map<std::string,Value>& value );

  template<typename Filter>
  inline void unpack( Filter& c, const mace::rpc::value& jsv, mace::rpc::value& v ) { v = jsv; }
  template<typename T, typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, const T& v ); 
  template<typename T, typename Filter> 
  void unpack( Filter& c, const mace::rpc::value& jsv, T& v ); 
  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, bool& v );

  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, void_t& v ){ 
    // TODO: perform sanity check...
  };

  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, float& v );
  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, double& v );
  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, uint8_t& v );
  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, uint16_t& v );
  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, uint32_t& v );
  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, uint64_t& v );
  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, int8_t& v );
  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, int16_t& v );
  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, int32_t& v );
  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, int64_t& v );
  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, std::string& v );


  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, std::vector<double>& value );

  template<typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, std::vector<char>& value );
  template<typename T, typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, std::vector<T>& value );
  template<typename T, typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, std::list<T>& value );

  template<typename T, typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, std::set<T>& value );
  template<typename Key, typename Value, typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, std::map<Key,Value>& value );
  template<typename Key, typename Value, typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, std::pair<Key,Value>& value );
  template<typename Value, typename Filter>
  void unpack( Filter& c, const mace::rpc::value& jsv, std::map<std::string,Value>& val );

  namespace detail {
    template<typename Class, typename Filter>
    struct pack_object_visitor {
      pack_object_visitor(Filter& _f, const Class& _c, mace::rpc::value& _val)
      :f(_f),c(_c),obj(_val){}

      /**
      VC++ does not understand the difference of return types, so an extra layer is needed.
      */
      template<typename T>
      inline void pack_helper( const T& v, const char* name )const {
        mace::rpc::pack( f, obj[name], f(v) ); 
      }
      template<typename T>
      inline void pack_helper( const boost::optional<T>& v, const char* name )const {
        if( !!v ) {
          mace::rpc::pack( f, obj[name], f(*v) ); 
        }
      }
      template<typename T, T  p>
      inline void operator()( const char* name )const {
        pack_helper( c.*p, name );
      }

      private:            
        Filter&            f;
        const Class&       c;
        mace::rpc::value& obj;
    };

    template<typename T>
    struct is_optional {
      typedef boost::false_type type;
    };
    template<typename T>
    struct is_optional<boost::optional<T> > {
      typedef boost::true_type type;
    };

    template<typename Class, typename Filter>
    struct unpack_object_visitor  {
      unpack_object_visitor(Filter& _f, Class& _c, const mace::rpc::value& _val)
      :f(_f),c(_c),obj(_val){}

      template<typename T, T p>
      void operator()( const char* name )const {
         if( obj.find(name) != obj.end()) {
             mace::rpc::unpack( f, obj[name], c.*p );
         }
         else {
            if( !is_optional< typename std::remove_reference<decltype(c.*p)>::type >::type::value ) {
                wlog( "unable to find name: '%1%'",name);
            }
         }
      }
      Filter&                 f;
      Class&                  c;
      const mace::rpc::value& obj;
    };

    template<typename Filter>
    struct pack_sequence {
       pack_sequence( Filter& _f, mace::rpc::array& _a ):f(_f),a(_a){}

       Filter&         f;
       mace::rpc::array&    a;
       
       template<typename T>
       void operator() ( const T& v )const {
          a.fields.push_back( mace::rpc::value());
          mace::rpc::pack(f,a.fields.back(),f(v));
       }
    };

    template<typename Filter>
    struct unpack_sequence {
       unpack_sequence( Filter& _f, const rpc::value& _val, int _i, int _size )
       :f(_f),val(_val),i(_i),s(_size){}

       Filter&                            f;
       const mace::rpc::value&            val;
       mutable int                        i;
       int                                s;
       
       template<typename T>
       void operator() ( T& v )const {
          if( i < s ) {
              apply_unpack_filter( f, val[i], v );
              ++i;
          }
       }
    };

    template<bool IsFusionSeq> struct if_fusion_seq {
      template<typename T,typename Filter> 
      inline static void pack( Filter& f, mace::rpc::value& jsv, const T& v ) {
          mace::rpc::array a;
          pack_sequence<Filter> pack_vector(f, a );
          boost::fusion::for_each( v, pack_vector );
          jsv = std::move(a);
      }
      template<typename T,typename Filter> 
      inline static void unpack( Filter& f, const mace::rpc::value& jsv, T& v ) {
          unpack_sequence<Filter> unpack_vector(f,jsv,0,jsv.size());
          boost::fusion::for_each( v, unpack_vector );
      }
    };

    template<typename IsReflected=boost::false_type>
    struct if_enum {
      template<typename T,typename Filter>
      static inline void pack( Filter& f, mace::rpc::value& jsv, const T& v ) { 
         jsv = mace::rpc::object();
         detail::pack_object_visitor<T,Filter> pov(f,f(v),jsv);
         mace::reflect::reflector<T>::visit(pov);
      }
      template<typename T,typename Filter>
      static inline void unpack( Filter& f, const mace::rpc::value& jsv, T& v ) { 
         detail::unpack_object_visitor<T,Filter> pov(f,v,jsv );
         mace::reflect::reflector<T>::visit(pov);
      }
    };

    template<>
    struct if_enum<boost::true_type> {
      template<typename T,typename Filter>
      static inline void pack( Filter& f, mace::rpc::value& jsv, const T& v ) { 
         mace::rpc::pack( f, jsv, mace::reflect::reflector<T>::to_string(v) );
      }
      template<typename T,typename Filter>
      static inline void unpack( Filter& f, const mace::rpc::value& jsv, T& v ) { 
         if( strcmp( jsv.type(), "string" ) == 0 ) {
            v = mace::reflect::reflector<T>::from_string( value_cast<std::string>(jsv).c_str() );
         } else {
            // throw if invalid int, by attempting to convert to string
            mace::reflect::reflector<T>::to_string( v = value_cast<int64_t>(jsv) );
         }
      }
    };


    template<typename IsReflected=boost::false_type>
    struct if_reflected {
      template<typename T,typename Filter>
      static inline void pack( Filter& f,mace::rpc::value& s, const T& v ) { 
       // wlog( "warning, ignoring unknown type" );
        v.did_not_implement_reflect_macro();
       // std::stringstream ss; ss << v;
       // mace::rpc::pack(f,s,f(mace::rpc::base64_encode((unsigned char const*)ss.str().c_str(),ss.str().size())));
      }
      //template<typename T,typename Filter>
      //static inline void unpack( Filter& f, const mace::rpc::value& s, T& v ) { 
      //  wlog( "warning, ignoring unknown type '%s'", reflect::get_typename<T>() );
       // std::string str;
       // mace::rpc::unpack(f,s,str);
       // std::stringstream ss(mace::rpc::base64_decode(str)); 
       // ss >> v;
      //}
    };
    template<>
    struct if_reflected<boost::true_type> {
      template<typename T,typename Filter>
      static inline void pack( Filter& f, mace::rpc::value& jsv, const T& v ) { 
         if_enum<typename mace::reflect::reflector<T>::is_enum>::pack( f,jsv,v );
      }
      template<typename T,typename Filter>
      static inline void unpack( Filter& f, const mace::rpc::value& jsv, T& v ) { 
         if_enum<typename mace::reflect::reflector<T>::is_enum>::unpack( f,jsv,v );
      }
    };

    template<> struct if_fusion_seq<false> {
        template<typename T,typename Filter> 
        inline static void pack( Filter& f, mace::rpc::value& jsv, const T& v ) {
            if_reflected<typename mace::reflect::reflector<T>::is_defined>::pack(f,jsv,f(v));
        }
        template<typename T,typename Filter> 
        inline static void unpack( Filter& f, const mace::rpc::value& jsv, T& v ) {
            if_reflected<typename mace::reflect::reflector<T>::is_defined>::unpack(f,jsv,v);
        }
    };

    /*
    template<> struct if_fusion_seq<false> {
        template<typename T> 
        inline static void pack( mace::rpc::value& jsv, const T& v ) {
            jsv = mace::rpc::object();
            detail::pack_object_visitor<T> pov(v,jsv);
            mace::reflect::reflector<T>::visit(pov);
        }
        template<typename T> 
        inline static void unpack( const mace::rpc::value& jsv, T& v ) {
            detail::unpack_object_visitor<T> pov(v,jsv );
            mace::reflect::reflector<T>::visit(pov);
        }
    };
    */
  } // namesapce detail

  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, bool& v )     { v = value_cast<bool>(jsv); }
  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, float& v )    { v = value_cast<float>(jsv); }
  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, double& v )   { v = value_cast<double>(jsv); }
  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, uint8_t& v )  { v = value_cast<uint8_t>(jsv); }
  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, uint16_t& v ) { v = value_cast<uint16_t>(jsv); }
  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, uint32_t& v )     { v = value_cast<uint32_t>(jsv); }
  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, uint64_t& v )     { v = value_cast<uint64_t>(jsv); }
  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, int8_t& v )       { v = value_cast<int8_t>(jsv); }
  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, int16_t& v )      { v = value_cast<int16_t>(jsv); }
  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, int32_t& v )      { v = value_cast<int32_t>(jsv); }
  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, int64_t& v )      { v = value_cast<int64_t>(jsv); }
  template<typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, std::string& v )  { v = value_cast<std::string>(jsv); }

  template<typename F, typename T>
  void apply_unpack_filter( F& f, const mace::rpc::value& s, T& t ) {
    if( f.is_filtered( &t ) ) {
      typename boost::remove_reference<decltype(f(T()))>::type r;
      mace::rpc::unpack( f, s, r );
      f( r, t );
    } else {
      mace::rpc::unpack( f, s, t );
    }
  }

  template<typename T, typename Filter> 
  void pack( Filter& f , mace::rpc::value& jsv, const boost::optional<T>& v ) {
    if( v ) pack( f, jsv, *v );
    else jsv = mace::rpc::value();
  }
  template<typename T, typename Filter> 
  void unpack( Filter& f, const mace::rpc::value& jsv, boost::optional<T>& v ) {
    if( strcmp( jsv.type(), "void" ) != 0 ) {
      T tmp;
      unpack( f, jsv, tmp );
      v = std::move(tmp);
    }
  }

  template<typename T, typename Filter> 
  inline void pack( Filter& f, mace::rpc::value& jsv, const T& v ) {
      detail::if_fusion_seq< boost::fusion::traits::is_sequence<T>::value >::pack(f,jsv,f(v));
  }

 /* 
  template<typename T, typename Filter> 
  inline void pack( Filter& f, mace::rpc::value& jsv, const boost::optional<T>& v ) {
      mace::rpc::pack( f, jsv, f(*v) );
  }

  template<typename T, typename Filter>
  inline void unpack( Filter& f, const mace::rpc::value& jsv, boost::optional<T>& v ) {
      v = T();
      mace::rpc::unpack( f, jsv, *v );
  }
  */


  template<typename T, typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const std::vector<T>& value ) {
      jsv = mace::rpc::array();
      jsv.resize(value.size());
      typename std::vector<T>::const_iterator itr = value.begin();
      typename std::vector<T>::const_iterator end = value.end();
      uint32_t i = 0;
      while( itr != end ) {
          mace::rpc::pack( f, jsv[i], f(*itr) );
          ++itr;
          ++i;
      }
  }

  template<typename T, typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const std::list<T>& value ) {
      jsv = mace::rpc::array();
      jsv.resize(value.size());
      typename std::list<T>::const_iterator itr = value.begin();
      typename std::list<T>::const_iterator end = value.end();
      uint32_t i = 0;
      while( itr != end ) {
          mace::rpc::pack( f, jsv[i], f(*itr) );
          ++itr;
          ++i;
      }
  }


  template<typename T, typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const std::set<T>& value ) {
      jsv = mace::rpc::array();
      jsv.resize(value.size());
      typename std::set<T>::const_iterator itr = value.begin();
      typename std::set<T>::const_iterator end = value.end();
      uint32_t i = 0;
      while( itr != end ) {
          mace::rpc::pack( f, jsv[i], f(*itr) );
          ++itr;
          ++i;
      }
  }

  template<typename T, typename Filter>
  inline void unpack( Filter& f, const mace::rpc::value& jsv, std::vector<T>& val ) {
      val.resize( jsv.size() );
      uint32_t s = jsv.size();
      for( uint32_t i = 0; i < s; ++i ) {
          apply_unpack_filter( f, jsv[i], val[i] );
      }
  }
  template<typename T, typename Filter>
  inline void unpack( Filter& f, const mace::rpc::value& jsv, std::list<T>& val ) {
      val.resize( jsv.size() );
      auto itr = val.begin();
      uint32_t s = jsv.size();
      for( uint32_t i = 0; i < s; ++i ) {
          apply_unpack_filter( f, jsv[i], *itr );
          ++itr;
      }
  }
  template<typename T, typename Filter>
  inline void unpack( Filter& f, const mace::rpc::value& jsv, std::set<T>& val ) {
      val.clear();
      uint32_t s = jsv.size();
      for( uint32_t i = 0; i < s; ++i ) {
          T v;
          apply_unpack_filter( f, jsv[i], v );
          val.insert(std::move(v));
      }
  }

  // support for pair!
  template<typename Key, typename Value, typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const std::pair<Key,Value>& val ) {
      jsv = mace::rpc::array(2);
      mace::rpc::pack( f, jsv[int(0)], f(val.first) );
      mace::rpc::pack( f, jsv[int(1)], f(val.second) );
  }
  // support for pair!
  template<typename Key, typename Value, typename Filter>
  void unpack( Filter& f, const mace::rpc::value& jsv, std::pair<Key,Value>& val ) {
      apply_unpack_filter( f, jsv[int(0)], val.first );
      apply_unpack_filter( f, jsv[int(1)], val.second );
  }


  // support arbitrary key/value containers as an array of pairs
  template<typename Key, typename Value, typename Filter>
  void pack( Filter& f, mace::rpc::value& jsv, const std::map<Key,Value>& val ) {
      jsv = mace::rpc::array();
      jsv.resize(val.size());
      typename std::map<Key,Value>::const_iterator itr = val.begin();
      typename std::map<Key,Value>::const_iterator end = val.end();
      uint32_t i = 0;
      while( itr != end ) {
          mace::rpc::pack( f, jsv[i], f(*itr) );
          ++itr;
          ++i;
      }
  }

  template<typename Key, typename Value, typename Filter>
  inline void unpack( Filter& f, const mace::rpc::value& jsv, std::map<Key,Value>& val ) {
      val.clear();
      for( uint32_t i = 0; i < jsv.size(); ++i ) {
          std::pair<Key,Value> p;
          mace::rpc::unpack( f, jsv[i], p );
          val[p.first] = std::move(p.second);
      }
  }

  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const std::vector<double>& data ) {
     if( data.size() ) { pack( f, jsv, f(mace::rpc::base64_encode((unsigned char*)&data.front(),data.size()*8))); } 
     else{ pack( f, jsv, "" ); }
  }
  template<typename Filter>
  inline void pack( Filter& f, mace::rpc::value& jsv, const std::vector<char>& data ) {
     if( data.size() ) { pack( f, jsv, f(mace::rpc::base64_encode((unsigned char*)&data.front(),data.size()))); } 
  }
  template<typename Filter>
  inline void unpack( Filter& f, const mace::rpc::value& jsv, std::vector<char>& data ) {
      data.clear();
      std::string d = mace::rpc::base64_decode( value_cast<std::string>(jsv) );
      data.insert(data.begin(),d.begin(),d.end());
  }
  template<typename Filter>
  inline void unpack( Filter& f, const mace::rpc::value& jsv, std::vector<double>& data ) {
      data.clear();
      std::string d = mace::rpc::base64_decode( value_cast<std::string>(jsv) );
      data.resize( d.size() / 8 );
      memcpy( (char*)&data.front(), d.c_str(), d.size() );
      //data.insert(data.begin(),d.begin(),d.end());
  }


  // pack map<string,T> as a JSON Object
  template<typename Value,typename Filter>
  void pack( Filter& f, mace::rpc::value& jsv, const std::map<std::string,Value>& val ) {
      mace::rpc::object o;
      o.fields.reserve(val.size());
      typename std::map<std::string,Value>::const_iterator itr = val.begin();
      typename std::map<std::string,Value>::const_iterator end = val.end();
      while( itr != end ) {
          mace::rpc::pack( f, o[itr->first], f(itr->second) );
          ++itr;
      }
      jsv = std::move(o);
  }
  template<typename Value, typename Filter>
  void unpack( Filter& f, const mace::rpc::value& jsv, std::map<std::string,Value>& val ) {
      val.clear();
      auto e = jsv.end();
      for( auto i = jsv.begin(); i != e; ++i ) {
          apply_unpack_filter( f, i->val, val[i->key] );
      }
  }
  template<typename T, typename Filter> 
  inline void unpack( Filter& f, const mace::rpc::value& jsv, T& v ) {
      detail::if_fusion_seq< boost::fusion::traits::is_sequence<T>::value >::unpack(f, jsv,v);
  }


  template<typename T> 
  void pack( mace::rpc::value& jsv, const T& v )  {
    //function_filter<void> f;
    default_filter f;
    pack( f, jsv, v );
  }

  template<typename T> 
  void unpack( const mace::rpc::value& jsv, T& v ) {
    //function_filter<void> f;
    default_filter f;
    unpack( f, jsv, v );
  }
  template<typename T> 
  T unpack( const mace::rpc::value& jsv ) {
    T tmp;
    default_filter f;
    unpack( f, jsv, tmp );
    return tmp;
  }
    
} }  // mace::rpc

#endif // VALUE_IO_HPP
