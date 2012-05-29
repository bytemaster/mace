#ifndef _MACE_REFLECT_VALUE_BASE_HPP_
#define _MACE_REFLECT_VALUE_BASE_HPP_
#include <typeinfo>
#include <mace/reflect/value_visitor.hpp>

namespace mace { namespace reflect {
  namespace detail {
    class place_holder;
  };

class iterator;
class value;
class const_iterator;

/**
 *  @class value_base
 *  @brief Provides runtime access to class members.
 *
 *  This class serves as the base for value_ref, value_cref, and
 *  value which handle the copy and ownership semantics while value_base
 *  deals with the interface semantics.
 *
 *  @code
 *    struct test {
 *      int num;
 *      std::string str;
 *      int print( std::string& );
 *    };
 *    BOOST_REFLECT( test, (num)(str)(print) )
 *
 *    value v(test());
 *    v["num"].as<int>();
 *    v["num"].as<std::string>();
 *    v["str"].as<std::string>();
 *    v["str"].as<int>();
 *    v["print"]( "hello world" );
 *
 *  @endcode
 *
 *  Given a value you can iterate over its members and 
 *  perform actions.
 */
  class value_base {
    public:
      value_base();
      value_base( const value_base& );
      ~value_base();

    bool           is_array()const;
    bool           is_function()const;
    size_t         size()const;

    /**
     *  If a struct, iterates over fields
     *  If a map, over keys
     *  if an array, over indexes
     */
    iterator       begin();
    const_iterator begin()const;
    iterator       end();
    const_iterator end()const;

      /**
       *  If a function, calls it...
       */
      value operator()()const;
      value operator()();

      template<typename P1>
      value operator()(P1)const;
      template<typename P1>
      value operator()(P1);

      template<typename P1, typename P2>
      value operator()(P1,P2)const;
      template<typename P1,typename P2>
      value operator()(P1,P2);


      const char* type()const;
  
      void visit( read_value_visitor&& v )const;
      void visit( write_value_visitor&& v );

      template<typename T>
      T as()const;

      template<typename T>
      void set_as( const T&& );

      bool operator!()const;

      template<typename T>
      inline T& get();
      
      template<typename T>
      inline const T& get()const;

      template<typename T>
      inline const T* ptr()const; 

      template<typename T>
      inline T* ptr(); 

      bool       has_field( const std::string& field )const;

    protected:
      friend class value;
      friend class value_ref;
      friend class value_cref;
      inline const detail::place_holder* get_holder()const;
      inline       detail::place_holder* get_holder();

      char held[3*sizeof(void*)];
  };

} }

#endif
