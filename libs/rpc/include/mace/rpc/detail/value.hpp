#include <type_traits>
#include <stdexcept>
#include <vector>
#include <mace/reflect/reflect.hpp>

namespace mace { namespace rpc { 

  struct  key_val;
  struct  object;
  struct  array;
  class   value;

  struct object {
    typedef std::vector<key_val>::const_iterator const_iterator;

    std::string           type;
    std::vector<key_val>  fields;
  };
  struct array {
    std::string           type;
    std::vector<value>    fields;
  };


  namespace detail {
    struct const_visitor {
      virtual void operator()( const int8_t& v      ){};
      virtual void operator()( const int16_t& v     ){};
      virtual void operator()( const int32_t& v     ){};
      virtual void operator()( const int64_t& v     ){};
      virtual void operator()( const uint8_t& v     ){};
      virtual void operator()( const uint16_t& v    ){};
      virtual void operator()( const uint32_t& v    ){};
      virtual void operator()( const uint64_t& v    ){};
      virtual void operator()( const float& v       ){};
      virtual void operator()( const double& v      ){};
      virtual void operator()( const bool& v        ){};
      virtual void operator()( const std::string& v ){};
      virtual void operator()( const object&  ){};
      virtual void operator()( const array&  ){};
      virtual void operator()( ){};
    };
    struct visitor {
      virtual void operator()( int8_t& v      ){};
      virtual void operator()( int16_t& v     ){};
      virtual void operator()( int32_t& v     ){};
      virtual void operator()( int64_t& v     ){};
      virtual void operator()( uint8_t& v     ){};
      virtual void operator()( uint16_t& v    ){};
      virtual void operator()( uint32_t& v    ){};
      virtual void operator()( uint64_t& v    ){};
      virtual void operator()( float& v       ){};
      virtual void operator()( double& v      ){};
      virtual void operator()( bool& v        ){};
      virtual void operator()( std::string& v ){};
      virtual void operator()( object&  ){};
      virtual void operator()( array&  ){};
      virtual void operator()( ){};
    };

    struct value_holder {
      virtual ~value_holder();
      virtual const char* type()const;
      virtual void visit( const_visitor&& v )const;
      virtual void visit( visitor&& v );

      virtual void clear();
      virtual size_t size()const;
      virtual void resize( size_t );
      virtual void reserve( size_t );
      virtual value& at( size_t );
      virtual const value& at( size_t )const;
      virtual void push_back( value&& v );

      virtual value_holder* move_helper( char* c );
      virtual value_holder* copy_helper( char* c )const;
    };

    // fundamental values...
    template<typename T>
    struct value_holder_impl : value_holder {
      static_assert( std::is_fundamental<T>::value, "only fundamental types can be stored without specialization" );
      virtual const char* type()const             { return mace::reflect::get_typename<T>(); }
      virtual void visit( const_visitor&& v )const{ v(val); }
      virtual void visit( visitor&& v )           { v(val); }
      virtual void clear()                        { val = T(); }
      virtual size_t size()const                  { return 0;  }

      virtual value_holder* move_helper( char* c ){ return new(c) value_holder_impl( std::move(val) ); }
      virtual value_holder* copy_helper( char* c )const{ return new(c) value_holder_impl(val);              }

      template<typename V>
      value_holder_impl( V&& v ):val( std::forward<V>(v) ){}

      T val;
    };
    
    template<>
    struct value_holder_impl<std::string> : value_holder {
      template<typename V>
      value_holder_impl( V&& v ):val( std::forward<V>(v) ){}

      virtual const char* type()const              { return "string"; }
      virtual void visit( const_visitor&& v )const { v(val); }
      virtual void visit( visitor&& v )            { v(val); }

      virtual value_holder* move_helper( char* c ){ return new(c) value_holder_impl( std::move(val) ); }
      virtual value_holder* copy_helper( char* c )const{ return new(c) value_holder_impl(val);              }

      virtual void clear()                        { val = std::string(); }
      virtual size_t size()const                  { return 0;  }


      std::string val;
    };

    template<>
    struct value_holder_impl<object> : value_holder {
      virtual const char* type()const              { return "object"; }
      virtual void visit( const_visitor&& v )const;
      virtual void visit( visitor&& v );
      virtual value_holder* move_helper( char* c );
      virtual value_holder* copy_helper( char* c )const;
      virtual void reserve( size_t s );

      virtual void clear();
      virtual size_t size()const;

      template<typename V>
      value_holder_impl( V&& v ):val( std::forward<V>(v) ){}

      object val; 
    };

    template<>
    struct value_holder_impl<array> : value_holder {
      virtual const char* type()const              { return "array"; }
      virtual void visit( const_visitor&& v )const;
      virtual void visit( visitor&& v );
      virtual value_holder* move_helper( char* c );
      virtual value_holder* copy_helper( char* c )const;

      virtual void resize( size_t s );
      virtual void reserve( size_t s );
      virtual value& at( size_t i);
      virtual const value& at( size_t i)const;
      virtual void push_back( value&& v );

      template<typename V>
      value_holder_impl( V&& v ):val( std::forward<V>(v) ){}

      virtual void clear();
      virtual size_t size()const;
      
      array val; 
    };
} } } // mace::rpc::detail
