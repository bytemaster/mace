#ifndef _MACE_RPC_VALUE_HPP_
#define _MACE_RPC_VALUE_HPP_
#include <mace/rpc/detail/value.hpp>

namespace mace { namespace rpc {
    /**
     *  @brief a dynamic container that can hold
     *  integers, reals, strings, booleans, arrays, and
     *  or null.   
     *
     *  This type serves as an intermediate representation between
     *  C++ type and serialized type (JSON,XML,etc).  
     *
     *  As long as a type is reflected via Mace.Reflect, a common STL container, or
     *  a Boost.Fusion Sequence it can be converted to and from a value.
     *
     *  As much as possible value attempts to preserve 'type' information, but
     *  type information is not always provided equally by all serialization formats.
     *
     *  value is move aware, so move it when you can to avoid expensive copies
     */
    class value {
      public:
        value();
        value( value&& m );
        value( const value& m );
        value( value& m );
        value( char* c );
        value( int8_t );
        value( int16_t );
        value( int32_t );
        value( int64_t );
        value( uint8_t );
        value( uint16_t );
        value( uint32_t );
        value( uint64_t );
        value( double );
        value( float );
        value( bool );
        value( std::string&& );
        value( std::string& );
        value( const std::string& );

        value( object&& o );
        value( const object& o );
        value( object& o );

        value( array&& a );
        value( array& a );
        value( const array& a );

        ~value();

        value& operator=( value&& v );
        value& operator=( const value& v );

        template<typename T>
        value( T&& v );

        template<typename T>
        value& operator=( T&& v );

        /** used to iterate over object fields, use array index + size to iterate over array */
        object::const_iterator find( const char* key )const;
        object::const_iterator begin()const;
        object::const_iterator end()const;

        /** avoid creating temporary string just for comparisons! **/
        value&       operator[]( const char* key );
        const value& operator[]( const char* key )const;

        /** array & object interface **/
        void         clear();
        size_t       size()const;

        /** array interface **/
        void         resize( size_t s );
        void         reserve( size_t s );
        void         push_back( value&& v );
        value&       operator[]( int32_t idx );
        const value& operator[]( int32_t idx )const;

        /** gets the stored type **/
        const char*  type()const;


      private:
        /** throws exceptions on errors **/
        template<typename T>
        friend T value_cast( const value& v );

        char holder[sizeof(detail::value_holder_impl<object>)];
    };

    struct key_val {
      key_val(){};

      key_val( std::string k, value v = value())
      :key(std::move(k)),val(std::move(v)){}

      key_val( key_val&& m )
      :key(std::move(m.key)),val(std::move(m.val)){}

      key_val( const key_val& m )
      :key(m.key),val(m.val){}

      std::string key;
      value       val;
    };


} } // mace::rpc

#include <mace/rpc/detail/value.ipp>

#endif
