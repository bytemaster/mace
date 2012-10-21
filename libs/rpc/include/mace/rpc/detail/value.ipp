#include <mace/rpc/value.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/lexical_cast.hpp>
#include <mace/rpc/value_io.hpp>

namespace mace { namespace rpc {

    /**
     *  Converts T to a value using reflection if
     *  necessary.
     */
    template<typename T>
    value::value( T&& v ) {
      new (holder) detail::value_holder(); 
      mace::rpc::pack( *this, std::forward<T>(v) );
    }

    template<typename T>
    value& value::operator=( T&& v )
    {
      value tmp( std::forward<T>(v) );
      std::swap(*this,tmp);
      return *this;
    }

    template<typename T>
    struct cast_visitor : const_visitor {
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
      virtual void operator()( const bool& v        ){ m_out = 0 !=v; }
      virtual void operator()( const std::string& v ){ m_out = boost::lexical_cast<T>(v); }
      virtual void operator()( const object&  )      { throw std::bad_cast();             }
      virtual void operator()( const array&  )       { throw std::bad_cast();             }
      virtual void operator()( )                     { throw std::bad_cast();             }
      private:
      T& m_out;
    };

    template<>
    struct cast_visitor<std::string> : const_visitor {
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
      virtual void operator()( const bool& v        ){ m_out = m_out = v ? "true" : "false"; }
      virtual void operator()( const std::string& v ){ m_out = v;                                   }
      virtual void operator()( const object&  )      { throw std::bad_cast();                       }
      virtual void operator()( const array&  )       { throw std::bad_cast();                       }
      virtual void operator()( )                     { throw std::bad_cast();                       }

      private:
      std::string& m_out;
    };
    template<>
    struct cast_visitor<bool> : const_visitor {
      cast_visitor( bool& out )
      :m_out(out){}
      virtual void operator()( const int8_t& v      ){ m_out = v != 0; }
      virtual void operator()( const int16_t& v     ){ m_out = v != 0; }
      virtual void operator()( const int32_t& v     ){ m_out = v != 0; }
      virtual void operator()( const int64_t& v     ){ m_out = v != 0; }
      virtual void operator()( const uint8_t& v     ){ m_out = v != 0; }
      virtual void operator()( const uint16_t& v    ){ m_out = v != 0; }
      virtual void operator()( const uint32_t& v    ){ m_out = v != 0; }
      virtual void operator()( const uint64_t& v    ){ m_out = v != 0; }
      virtual void operator()( const float& v       ){ m_out = v != 0; }
      virtual void operator()( const double& v      ){ m_out = v != 0; }
      virtual void operator()( const bool& v        ){ m_out = v;                                    }
      virtual void operator()( const std::string& v ){ m_out = v != "false" && v != "0" && v.size(); }
      virtual void operator()( const object&  )      { throw std::bad_cast();                        }
      virtual void operator()( const array&  )       { throw std::bad_cast();                        }
      virtual void operator()( )                     { throw std::bad_cast();                        }

      private:
      bool& m_out;
    };

    template<>
    struct cast_visitor<array> : const_visitor {
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
    struct cast_visitor<object> : const_visitor {
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
    template<>
    struct cast_visitor<void> : const_visitor {
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
      virtual void operator()( const object& a )     { throw std::bad_cast();}
      virtual void operator()( const array&  )       { throw std::bad_cast();}
      virtual void operator()( )                     { }
    };

    namespace detail {
        template<bool IsSeq=false>
        struct cast_if_seq {
          template<typename T>
          static T cast( detail::value_holder* h, const value& v ) {
             T out;
             h->visit(cast_visitor<T>(out));
             return out;
          }
        };
        template<>
        struct cast_if_seq<true> {
          template<typename T>
          static T cast( detail::value_holder* h, const value& v ) {
             return unpack<T>(v);
          }
        };

        template<typename IsReflected=boost::false_type>
        struct cast_if_reflected {
          template<typename T>
          static T cast( detail::value_holder* h, const value& v ) {
             return detail::cast_if_seq<boost::fusion::traits::is_sequence<T>::value>::template cast<T>(h,v);
             /*
             T out;
             h->visit(cast_visitor<T>(out));
             return out;
             */
          }
        };

        template<>
        struct cast_if_reflected<boost::true_type> {
          template<typename T>
          static T cast( detail::value_holder* h, const value& v ) {
             return unpack<T>(v);
          }
        };
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
      auto h = ((detail::value_holder*)&v.holder[0]);
      return detail::cast_if_reflected<typename mace::reflect::reflector<T>::is_defined>::template cast<T>(h,v);
    }



} } 
