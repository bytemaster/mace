#ifndef _MACE_RPC_RAW_RAW_IO_HPP_
#define _MACE_RPC_RAW_RAW_IO_HPP_
#include <mace/rpc/filter.hpp>
#include <mace/reflect/reflect.hpp>
#include <sstream>
#include <iostream>
#include <map>
#include <iomanip>
#include <mace/rpc/datastream.hpp>
#include <sstream>

#include <boost/optional.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>

namespace mace { namespace rpc { namespace raw { 
    template<typename F, typename S, typename T>
    void apply_unpack_filter( F& f, S& s, T& t ); 

    template<typename T, typename Filter, typename Stream> 
    void pack( Filter&, Stream& st, const T& v ); 

    template<typename T, typename Filter, typename Stream> 
    void unpack( Filter&, Stream& st, T& v ); 

    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const bool& v )         { char c(v?1:0); st.write(&c,sizeof(c)); }
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const float& v )        { st.write( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const double& v )       { st.write( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const uint8_t& v )      { st.write( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const uint16_t& v )     { st.write( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const uint32_t& v )     { st.write( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const uint64_t& v )     { st.write( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const int8_t& v )       { st.write( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const int16_t& v )      { st.write( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const int32_t& v )      { st.write( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const int64_t& v )      { st.write( (char*)&v, sizeof(v) ); }

    template<typename Filter, typename Stream> 
    inline void pack( Filter& f, Stream& s, const signed_int& v ) {
      uint32_t val = (v.value<<1) ^ (v.value>>31);
      do {
        uint8_t b = uint8_t(val) & 0x7f;
        val >>= 7;
        b |= ((val > 0) << 7);
        s.write((char*)&b,1);
      } while( val );
    }
    template<typename Filter, typename Stream> 
    inline void pack( Filter& f, Stream& s, const unsigned_int& v ) {
      uint64_t val = v.value;
      do {
        uint8_t b = uint8_t(val) & 0x7f;
        val >>= 7;
        b |= ((val > 0) << 7);
        s.write((char*)&b,1);
      }while( val );
    }


    /** strings are packed as unsigned_int (size) followed by size bytes */
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const std::string& v )  {
      pack( f, st, unsigned_int(v.size())  );
      if( v.size() ) st.write( v.c_str(), v.size() );
    }
    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const char* v ) {
      pack( f, st, std::string(v) );
    }

    template<typename Filter, typename Stream>
    void pack( Filter& c, Stream& st, const std::vector<char>& value );
    template<typename T, typename Filter, typename Stream>
    void pack( Filter& c, Stream& st, const boost::optional<T>& v );
    template<typename T, typename Filter, typename Stream>
    void pack( Filter& c, Stream& st, const std::vector<T>& value );
    template<typename T, typename Filter, typename Stream>
    void pack( Filter& c, Stream& st, const std::list<T>& value );

    template<typename T, typename Filter, typename Stream>
    void pack( Filter& c, Stream& st, const std::set<T>& value );
    template<typename Key, typename Value, typename Filter, typename Stream>
    void pack( Filter& c, Stream& st, const std::map<Key,Value>& value );
    template<typename Key, typename Value, typename Filter, typename Stream>
    void pack( Filter& c, Stream& st, const std::pair<Key,Value>& value );
    template<typename Value, typename Filter, typename Stream>
    void pack( Filter& c, Stream& st, const std::map<std::string,Value>& value );


    template<typename Filter, typename Stream> inline void unpack( Filter&, Stream& s, signed_int& vi ) {
      uint32_t v = 0; char b = 0; int by = 0;
      do {
        s.read(&b,sizeof(b));
        v |= uint32_t(uint8_t(b) & 0x7f) << by;
        by += 7;
      } while( uint8_t(b) & 0x80 );
      vi.value = ((v>>1) ^ (v>>31)) + (v&0x01);
      vi.value = v&0x01 ? vi.value : -vi.value;
      vi.value = -vi.value;
    }
    template<typename Filter, typename Stream> inline void unpack( Filter&, Stream& s, unsigned_int& vi ) {
      uint64_t v = 0; char b = 0; uint8_t by = 0;
      do {
          s.read(&b,sizeof(b));
          v |= uint32_t(uint8_t(b) & 0x7f) << by;
          by += 7;
      } while( uint8_t(b) & 0x80 );
      vi.value = v;
    }

    template<typename T, typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, const T& v ); 
    template<typename T, typename Filter, typename Stream> 
    void unpack( Filter& c, Stream& st, T& v ); 
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, bool& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, float& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, double& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, uint8_t& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, uint16_t& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, uint32_t& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, uint64_t& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, int8_t& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, int16_t& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, int32_t& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, int64_t& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, std::string& v );
    template<typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, std::vector<char>& value );
    template<typename T, typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, boost::optional<T>& v );
    template<typename T, typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, std::vector<T>& value );
    template<typename T, typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, std::list<T>& value );

    template<typename T, typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, std::set<T>& value );
    template<typename Key, typename Value, typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, std::map<Key,Value>& value );
    template<typename Key, typename Value, typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, std::pair<Key,Value>& value );
    template<typename Value, typename Filter, typename Stream>
    void unpack( Filter& c, Stream& st, std::map<std::string,Value>& val );

    namespace detail {
      template<typename Class, typename Filter, typename Stream>
      struct pack_object_visitor {
        pack_object_visitor(Filter& _f, const Class& _c, Stream& _s)
        :f(_f),c(_c),s(_s){}

        /**
        VC++ does not understand the difference of return types, so an extra layer is needed.
        */
        template<typename T>
        inline void pack_helper( const T& v, const char* name )const {
          raw::pack( f, s, f(v) ); 
        }
        template<typename T>
        inline void pack_helper( const boost::optional<T>& v, const char* name )const {
          if( !!v ) {
            raw::pack( f, s, f(*v) ); 
          }
        }
        template<typename T, T  p>
        inline void operator()( const char* name )const {
          pack_helper( c.*p, name );
        }

        private:            
          Filter&       f;
          const Class&  c;
          Stream&       s;
      };

      template<typename Class, typename Filter, typename Stream>
      struct unpack_object_visitor  {
        unpack_object_visitor(Filter& _f, Class& _c, Stream& _s)
        :f(_f),c(_c),s(_s){}

        template<typename T, T p>
        void operator()( const char* name )const {
           apply_unpack_filter( f, s, c.*p );
        }
        Filter&             f;
        Class&              c;
        Stream&             s;
      };

      template<typename Filter, typename Stream>
      struct pack_sequence {
         pack_sequence( Filter& _f, Stream& _s ):f(_f),s(_s){}

         Filter&    f;
         Stream&    s;
         
         template<typename T>
         void operator() ( const T& v )const {
            mace::rpc::raw::pack(f,s,f(v));
         }
      };

      template<typename Filter, typename Stream>
      struct unpack_sequence {
         unpack_sequence( Filter& _f, Stream& _s ):f(_f),s(_s){}

         Filter&  f;
         Stream&  s;
         
         template<typename T>
         void operator() ( T& v )const {
          apply_unpack_filter( f, s, v );
         }
      };

      template<bool IsFusionSeq> struct if_fusion_seq {
        template<typename T,typename Filter, typename Stream> 
        inline static void pack( Filter& f, Stream& st, const T& v ) {
            pack_sequence<Filter,Stream> pack_vector(f, st);
            boost::fusion::for_each( v, pack_vector );
        }
        template<typename T,typename Filter, typename Stream> 
        inline static void unpack( Filter& f, Stream& st, T& v ) {
            unpack_sequence<Filter,Stream> unpack_vector(f,st);
            boost::fusion::for_each( v, unpack_vector );
        }
      };

      template<typename IsReflected=boost::false_type>
      struct if_reflected {
        template<typename T,typename Filter, typename Stream>
        static inline void pack( Filter& f,Stream& s, const T& v ) { 
          //BOOST_STATIC_ASSERT_MSG( false, "Unknown type, not reflected" );
          elog( "Unknown type %1%", mace::reflect::get_typename<T>() );
          // TODO: Boost Serialize??
          //std::stringstream ss; ss << v;
          // TO BASE 64
          //raw::pack(f,s,f(mace::rpc::base64_encode((unsigned char const*)ss.str().c_str(),ss.str().size())));
        }
        template<typename T,typename Filter, typename Stream>
        static inline void unpack( Filter& f, Stream& s, T& v ) { 
          elog( "Unknown type %1%", mace::reflect::get_typename<T>() );
          //BOOST_STATIC_ASSERT_MSG( false, "Unknown type, not reflected" );
          // TODO: Boost Deserialize??
          //std::string str;
          //raw::unpack(f,s,str);
          //std::stringstream ss(mace::rpc::base64_decode(str)); 
          //ss >> v;
        }
      };
      template<>
      struct if_reflected<boost::true_type> {
        template<typename T,typename Filter, typename Stream>
        static inline void pack( Filter& f, Stream& st, const T& v ) { 
          detail::pack_object_visitor<T,Filter,Stream> pov(f,f(v),st);
          mace::reflect::reflector<T>::visit(pov);
        }
        template<typename T,typename Filter, typename Stream>
        static inline void unpack( Filter& f, Stream& st, T& v ) { 
          detail::unpack_object_visitor<T,Filter,Stream> pov(f,v,st );
          mace::reflect::reflector<T>::visit(pov);
        }
      };

      template<> struct if_fusion_seq<false> {
         template<typename T,typename Filter, typename Stream> 
         inline static void pack( Filter& f, Stream& st, const T& v ) {
           if_reflected<typename mace::reflect::reflector<T>::is_defined>::pack(f,st,v);
         }
         template<typename T,typename Filter, typename Stream> 
         inline static void unpack( Filter& f, Stream& st, T& v ) {
           if_reflected<typename mace::reflect::reflector<T>::is_defined>::unpack(f,st,v);
         }
      };

    } // namesapce detail

    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, bool& v )         { v = st.get(); }
    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, float& v )        { st.read( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, double& v )       { st.read( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, uint8_t& v )      { st.read( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, uint16_t& v )     { st.read( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, uint32_t& v )     { st.read( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, uint64_t& v )     { st.read( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, int8_t& v )       { st.read( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, int16_t& v )      { st.read( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, int32_t& v )      {st.read( (char*)&v, sizeof(v) ); }
    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, int64_t& v )      { st.read( (char*)&v, sizeof(v) ); }

    template<typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, std::string& v )  { 
      unsigned_int i;
      unpack( f, st, i );
      v.resize(i.value);
      if( i.value ) {
        st.read( const_cast<char*>(v.c_str()), i );
      }
    }

    template<typename T, typename Filter, typename Stream> 
    inline void pack( Filter& f, Stream& st, const T& v ) {
        detail::if_fusion_seq< boost::fusion::traits::is_sequence<T>::value >::pack(f,st,v);
    }

    
    template<typename T, typename Filter, typename Stream> 
    inline void pack( Filter& f, Stream& st, const boost::optional<T>& v ) {
        raw::pack( f, st, f(*v) );
    }

    template<typename T, typename Filter, typename Stream>
    inline void unpack( Filter& f, Stream& st, boost::optional<T>& v ) {
        v = T();
        raw::unpack( f, st, *v );
    }


    template<typename T, typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const std::vector<T>& value ) {
        pack( f, st, unsigned_int(value.size()) );
        auto itr = value.begin();
        auto end = value.end();
        while( itr != end ) {
          raw::pack( f, st, f(*itr) );
          ++itr;
        }
    }

    template<typename T, typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const std::list<T>& value ) {
        pack( f, st, unsigned_int(value.size()) );
        auto itr = value.begin();
        auto end = value.end();
        while( itr != end ) {
          raw::pack( f, st, f(*itr) );
          ++itr;
        }
    }


    template<typename T, typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const std::set<T>& value ) {
        pack( f, st, unsigned_int(value.size()) );
        auto itr = value.begin();
        auto end = value.end();
        while( itr != end ) {
          raw::pack( f, st, f(*itr) );
          ++itr;
        }
    }

    template<typename T, typename Filter, typename Stream>
    inline void unpack( Filter& f, Stream& st, std::vector<T>& value ) {
        unsigned_int s;
        unpack( f, st, s );
        value.resize(s.value);
        for( uint32_t i = 0; i < value.size(); ++i ) {
          raw::unpack( f, st, value[i] );
        }
    }
    template<typename T, typename Filter, typename Stream>
    inline void unpack( Filter& f, Stream& st, std::list<T>& value ) {
        unsigned_int s;
        unpack( f, st, s );
        value.clear();
        for( uint32_t i = 0; i < s.value; ++i ) {
            T v;
            raw::unpack( f, st, v );
            value.push_back(std::move(v));
        }
    }
    template<typename T, typename Filter, typename Stream>
    inline void unpack( Filter& f, Stream& st, std::set<T>& value ) {
        unsigned_int s;
        unpack( f, st, s );
        for( uint32_t i = 0; i < s.value; ++i ) {
            T v;
            raw::unpack( f, st, v );
            value.insert(std::move(v));
        }
    }

    // support for pair!
    template<typename Key, typename Value, typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const std::pair<Key,Value>& val ) {
        pack( f, st, f(val.first) );
        pack( f, st, f(val.second) );
    }
    // support for pair!
    template<typename Key, typename Value, typename Filter, typename Stream>
    void unpack( Filter& f, Stream& st, std::pair<Key,Value>& val ) {
        apply_unpack_filter( f, st, val.first );
        apply_unpack_filter( f, st, val.second );
    }


    // support arbitrary key/value containers as an array of pairs
    template<typename Key, typename Value, typename Filter, typename Stream>
    void pack( Filter& f, Stream& st, const std::map<Key,Value>& value ) {
        pack( f, st, unsigned_int(value.size()) );
        auto itr = value.begin();
        auto end = value.end();
        while( itr != end ) {
          raw::pack( f, st, f(*itr) );
          ++itr;
        }
    }

    template<typename Key, typename Value, typename Filter, typename Stream>
    inline void unpack( Filter& f, Stream& st, std::map<Key,Value>& value ) {
        value.clear();
        unsigned_int s;
        unpack( f, st, s );
        for( uint32_t i = 0; i < s.value; ++i ) {
          Key k;
          Value v;
          apply_unpack_filter( f, st, k );
          apply_unpack_filter( f, st, v );
          value[k] = std::move(v);
        }
    }

    template<typename Filter, typename Stream>
    inline void pack( Filter& f, Stream& st, const std::vector<char>& data ) {
       unsigned_int i(data.size());
       pack( f, st, i );
       if( data.size() )
          st.write(&data.front(),data.size());
    }
    template<typename Filter, typename Stream>
    inline void unpack( Filter& f, Stream& st, std::vector<char>& data ) {
        unsigned_int s;
        unpack(f,st,s);
        /// TODO: what if value is HUGE
        data.resize(s.value);
        if( s.value )
            st.read( &data.front(), data.size() );
    }

    template<typename T, typename Filter, typename Stream> 
    inline void unpack( Filter& f, Stream& st, T& v ) {
        detail::if_fusion_seq< boost::fusion::traits::is_sequence<T>::value >::unpack(f, st,v);
    }


    template<typename Stream, typename T> 
    void pack( Stream& st, const T& v )  {
      mace::rpc::default_filter f;
      pack( f, st, v );
    }

    template<typename Stream, typename T> 
    void unpack( Stream& st, T& v ) {
      mace::rpc::default_filter f;
      unpack( f, st, v );
    }

    template<typename F, typename S, typename T>
    void apply_unpack_filter( F& f, S& s, T& t ) {
      if( f.is_filtered( &t ) ) {
        typename boost::remove_reference<decltype(f(T()))>::type r;
        unpack( f, s, r );
        f( r, t );
      } else {
        unpack( f, s, t );
      }
    }
    
}  // namespace raw

  struct raw_io {
    template<typename T, typename Filter>
    static datavec pack( Filter& f, const T& v ) {
      std::stringstream ss;
      raw::pack( f, ss, v );
      std::string s = ss.str();
      datavec rv(s.size());
      if( s.size() )
        memcpy(  &rv.front(), s.c_str(), s.size());
      return rv;
    }
    template<typename T, typename Filter>
    static T unpack( Filter& f, const datavec& d ) {
      T tmp;
      datastream<const char*> ds(&d.front(),d.size());
      raw::unpack( f, ds, tmp );
      return tmp;
    }
  };

} } // mace::rpc

#endif // MACE_RPC_RAW_RAW_IO_HPP
