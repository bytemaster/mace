#ifndef __MACE_RPC_JSON_NAMED_PARAMETERS_HPP_
#define __MACE_RPC_JSON_NAMED_PARAMETERS_HPP_
/*
#include <boost/mpl/if.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
*/

namespace mace { namespace rpc { namespace json {

  /**
   *  @brief specialize this template for your type to
   *         indicate that the struct is a 'named parameter' type.
   */
  template<typename T>
  struct has_named_parameters{
    typedef boost::false_type type; 
    enum value_enum { value = 0 };
  };

} } }

#define MACE_RPC_JSON_NAMED_PARAMETERS( TYPE ) \
namespace mace { namespace rpc { namespace json { \
  template<> struct has_named_parameters<TYPE&>{ \
    typedef boost::true_type type;  \
    enum value_enum { value = 1 }; \
  };  \
  template<> struct has_named_parameters<TYPE&&>{ \
    typedef boost::true_type type;  \
    enum value_enum { value = 1 }; \
  };  \
  template<> struct has_named_parameters<const TYPE&>{ \
    typedef boost::true_type type;  \
    enum value_enum { value = 1 }; \
  };  \
  template<> struct has_named_parameters<const TYPE>{ \
    typedef boost::true_type type;  \
    enum value_enum { value = 1 }; \
  };  \
  template<> struct has_named_parameters<TYPE>{ \
    typedef boost::true_type type;  \
    enum value_enum { value = 1 }; \
  };  \
} } }


#endif // __MACE_RPC_JSON_NAMED_PARAMETERS_HPP_
