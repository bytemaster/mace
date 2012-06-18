#include <mace/rpc/value.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>

namespace mace { namespace rpc {

  namespace detail {
      void value_holder_impl<array>::resize( size_t s )               { val.fields.resize(s);  }
      void value_holder_impl<array>::reserve( size_t s )              { val.fields.reserve(s); }
      value& value_holder_impl<array>::at( size_t i)                  { return val.fields[i]; }
      const value& value_holder_impl<array>::at( size_t i)const       { return val.fields[i]; }
      value_holder* value_holder_impl<array>::move_helper( char* c ){ return new(c) value_holder_impl( std::move(val) ); }
      value_holder* value_holder_impl<array>::copy_helper( char* c )const{ return new(c) value_holder_impl(val);              }

      void value_holder_impl<array>::clear()                        { val.fields.clear();        }
      size_t value_holder_impl<array>::size()const                  { return val.fields.size();  }
      void value_holder_impl<array>::visit( const_visitor&& v )const { v(val); }
      void value_holder_impl<array>::visit( visitor&& v )            { v(val); }
      void value_holder_impl<array>::push_back( value&& v )          { val.fields.push_back( std::move(v) ); }


      void value_holder_impl<object>::visit( const_visitor&& v )const { v(val); }
      void value_holder_impl<object>::visit( visitor&& v )            { v(val); }
      value_holder* value_holder_impl<object>::move_helper( char* c ) { return new(c) value_holder_impl( std::move(val) ); }
      value_holder* value_holder_impl<object>::copy_helper( char* c )const { return new(c) value_holder_impl(val);              }
      void value_holder_impl<object>::reserve( size_t s )             { val.fields.reserve(s); }

      void value_holder_impl<object>::clear()                         { val = object(); }
      size_t value_holder_impl<object>::size()const                   { return val.fields.size();  }


    template<typename T, bool IsReflected=false, bool IsSeq=false>
    struct deduce_holder {
      typedef value_holder_impl<T> type;
    };

    template<>
    struct deduce_holder< std::string> {
      typedef value_holder_impl<std::string> type;
    };

    template<>
    struct deduce_holder< array > {
      typedef value_holder_impl<array> type;
    };

    template<>
    struct deduce_holder< object > {
      typedef value_holder_impl<object> type;
    };

    template<typename T>
    struct deduce_holder< std::vector<T> > {
      typedef value_holder_impl<array> type;
    };

    template<typename T>
    struct deduce_holder< std::list<T> > {
      typedef value_holder_impl<array> type;
    };

    template<typename T>
    struct deduce_holder< std::set<T> > {
      typedef value_holder_impl<array> type;
    };

    template<typename K, typename V>
    struct deduce_holder< std::pair<K,V> > {
      typedef value_holder_impl<array> type;
    };

    template<typename K, typename V>
    struct deduce_holder< std::map<K,V> > {
      typedef value_holder_impl<array> type;
    };

    template<typename V>
    struct deduce_holder< std::map<std::string,V> > {
      typedef value_holder_impl<object> type;
    };

    template<typename Sequence>
    struct deduce_holder<Sequence,false,true> {
      typedef value_holder_impl<array> type;
    };

    template<typename Object>
    struct deduce_holder<Object,true,false> {
      typedef value_holder_impl<object> type;
    };
  } // namespace detail

    /**
     *  Converts T to a value using reflection if
     *  necessary.
     */
    template<typename T>
    value::value( T&& v ) {
      typedef typename detail::deduce_holder<T,
                mace::reflect::reflector<T>::is_defined::value,
                boost::fusion::traits::is_sequence<T>::value>::type  holder_type;
      holder_type* ht = new (holder) holder_type( std::forward<T>(v) );
    }

    template<typename T>
    value& value::operator=( T&& v )
    {
      value tmp( std::forward<T>(v) );
      std::swap(*this,tmp);
      return *this;
    }

    template<typename T>
    struct cast_visitor : detail::const_visitor {
      cast_visitor( T& out )
      :m_out(out){}
      virtual void operator()( const int8_t& v      ){ m_out = boost::numeric_cast<T>(v); }
      virtual void operator()( const int16_t& v     ){ m_out = boost::numeric_cast<T>(v); }
      virtual void operator()( const int32_t& v     ){ m_out = boost::numeric_cast<T>(v); }
      virtual void operator()( const int64_t& v     ){ m_out = boost::numeric_cast<T>(v); }
      virtual void operator()( const uint8_t& v     ){ m_out = boost::numeric_cast<T>(v); }
      virtual void operator()( const uint16_t& v    ){ m_out = boost::numeric_cast<T>(v); }
      virtual void operator()( const uint32_t& v    ){ m_out = boost::numeric_cast<T>(v); }
      virtual void operator()( const uint64_t& v    ){ m_out = boost::numeric_cast<T>(v); }
      virtual void operator()( const float& v       ){ m_out = boost::numeric_cast<T>(v); }
      virtual void operator()( const double& v      ){ m_out = boost::numeric_cast<T>(v); }
      virtual void operator()( const bool& v        ){ m_out = boost::numeric_cast<T>(v); }
      virtual void operator()( const std::string& v ){ m_out = boost::lexical_cast<T>(v); }
      virtual void operator()( const object&  )      { throw std::bad_cast();             }
      virtual void operator()( const array&  )       { throw std::bad_cast();             }
      virtual void operator()( )                     { throw std::bad_cast();             }
      private:
      T& m_out;
    };

    template<>
    struct cast_visitor<std::string> : detail::const_visitor {
      cast_visitor( std::string& out )
      :m_out(out){}
      virtual void operator()( const int8_t& v      ){ m_out = boost::lexical_cast<std::string>(v); }
      virtual void operator()( const int16_t& v     ){ m_out = boost::lexical_cast<std::string>(v); }
      virtual void operator()( const int32_t& v     ){ m_out = boost::lexical_cast<std::string>(v); }
      virtual void operator()( const int64_t& v     ){ m_out = boost::lexical_cast<std::string>(v); }
      virtual void operator()( const uint8_t& v     ){ m_out = boost::lexical_cast<std::string>(v); }
      virtual void operator()( const uint16_t& v    ){ m_out = boost::lexical_cast<std::string>(v); }
      virtual void operator()( const uint32_t& v    ){ m_out = boost::lexical_cast<std::string>(v); }
      virtual void operator()( const uint64_t& v    ){ m_out = boost::lexical_cast<std::string>(v); }
      virtual void operator()( const float& v       ){ m_out = boost::lexical_cast<std::string>(v); }
      virtual void operator()( const double& v      ){ m_out = boost::lexical_cast<std::string>(v); }
      virtual void operator()( const bool& v        ){ m_out = boost::lexical_cast<std::string>(v); }
      virtual void operator()( const std::string& v ){ m_out = v;                                   }
      virtual void operator()( const object&  )      { throw std::bad_cast();                       }
      virtual void operator()( const array&  )       { throw std::bad_cast();                       }
      virtual void operator()( )                     { throw std::bad_cast();                       }

      private:
      std::string& m_out;
    };

    template<>
    struct cast_visitor<array> : detail::const_visitor {
      cast_visitor( array& out )
      :m_out(out){}
      virtual void operator()( const int8_t& v      ){ throw std::bad_cast();}
      virtual void operator()( const int16_t& v     ){ throw std::bad_cast();}
      virtual void operator()( const int32_t& v     ){ throw std::bad_cast();}
      virtual void operator()( const int64_t& v     ){ throw std::bad_cast();}
      virtual void operator()( const uint8_t& v     ){ throw std::bad_cast();}
      virtual void operator()( const uint16_t& v    ){ throw std::bad_cast();}
      virtual void operator()( const uint32_t& v    ){ throw std::bad_cast();}
      virtual void operator()( const uint64_t& v    ){ throw std::bad_cast();}
      virtual void operator()( const float& v       ){ throw std::bad_cast();}
      virtual void operator()( const double& v      ){ throw std::bad_cast();}
      virtual void operator()( const bool& v        ){ throw std::bad_cast();}
      virtual void operator()( const std::string& v ){ throw std::bad_cast();}
      virtual void operator()( const object&  )      { throw std::bad_cast();}
      virtual void operator()( const array& a )      { m_out = a;            }
      virtual void operator()( )                     { throw std::bad_cast();}

      private:
      array& m_out;
    };

    template<>
    struct cast_visitor<object> : detail::const_visitor {
      cast_visitor( object& out )
      :m_out(out){}
      virtual void operator()( const int8_t& v      ){ throw std::bad_cast();}
      virtual void operator()( const int16_t& v     ){ throw std::bad_cast();}
      virtual void operator()( const int32_t& v     ){ throw std::bad_cast();}
      virtual void operator()( const int64_t& v     ){ throw std::bad_cast();}
      virtual void operator()( const uint8_t& v     ){ throw std::bad_cast();}
      virtual void operator()( const uint16_t& v    ){ throw std::bad_cast();}
      virtual void operator()( const uint32_t& v    ){ throw std::bad_cast();}
      virtual void operator()( const uint64_t& v    ){ throw std::bad_cast();}
      virtual void operator()( const float& v       ){ throw std::bad_cast();}
      virtual void operator()( const double& v      ){ throw std::bad_cast();}
      virtual void operator()( const bool& v        ){ throw std::bad_cast();}
      virtual void operator()( const std::string& v ){ throw std::bad_cast();}
      virtual void operator()( const object& a )     { m_out = a;            }
      virtual void operator()( const array&  )       { throw std::bad_cast();}
      virtual void operator()( )                     { throw std::bad_cast();}

      private:
      object& m_out;
    };


    /**
     *  Convert from value v to T
     *
     *  Performs the following conversions
     *  true -> 1.0, 1, "true"
     *
     *  Not all casts are 'clean', the following conversions
     *  could cause errors:
     *
     *  signed int -> unsigned 
     *  large int -> smaller int
     *  real -> int
     *  non-numeric string -> number
     *  object -> string or number
     *  array -> string or number
     *  number,string,array -> object
     */
    template<typename T>
    T value_cast( const value& v ) {
      T out;
      detail::value_holder* vh = ((detail::value_holder*)&v.holder[0])->visit(cast_visitor<T>(out));
      return out;
    }



} } 
