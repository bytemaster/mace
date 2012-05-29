/**
 *  @file vtable.hpp
 *
 *  This class defines the macros and types used to implement
 *  a STUB interface.
 *
 */
#ifndef _MACE_STUB_VTABLE_HPP_
#define _MACE_STUB_VTABLE_HPP_
#include <boost/typeof/typeof.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/seq/push_front.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/seq/to_tuple.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#include <boost/preprocessor/list/enum.hpp>
#include <boost/preprocessor/stringize.hpp>

namespace mace { namespace stub {

  struct mirror_interface;
  
  /**
   *  @brief Contains functors defined by InterfaceDelegate for each reflected member of InterfaceType
   *
   *  Use the @ref MACE_STUB(NAME,MEMBERS) or MACE_STUB_DERIVED(NAME,BASES,MEMBERS) to define the vtable for your
   *  type.
   */
  template<typename InterfaceType = void, typename InterfaceDelegate = mace::stub::mirror_interface>
  class vtable {};
  
  /**
   *  @brief Enables specialization of visit for InterfaceType
   *
   *  This class is specialized by MACE_STUB and MACE_STUB_DERIVED to visit
   *  all stub methods.
   */
  template<typename InterfaceType,typename InterfaceDelegate = mace::stub::mirror_interface> 
  struct vtable_reflector {
    template<typename Visitor>
    static void visit( const Visitor& v ) {}
  };

#ifndef DOXYGEN
  /** used because MACE_STUB_SEQ_ENUM didn't quite work, and we need to
   * swollow an extra comma... perhaps there is something we could put here later */
  template<typename t>
  class vtable_base{};
#endif

} } // namespace mace::stub

#ifndef DOXYGEN

#define MACE_STUB_VTABLE_PUBLIC_BASE( r, data, elem )  mace::stub::vtable<elem,data>,

#define MACE_STUB_VTABLE_DEFINE_MEMBER( r, data, elem ) \
  struct BOOST_PP_CAT( __stub__, elem) : \
    public InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&interface_type::elem)>::type  { \
      typedef typename InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&interface_type::elem)>::type base_type;  \
      using base_type::operator=;\
      template<typename Type>\
      static decltype(&Type::elem) get_member_ptr() { return &Type::elem; } \
  } elem;


#define MACE_STUB_VTABLE_VISIT_BASE( r, visitor, name ) \
  vtable_reflector<name,InterfaceDelegate>::visit( visitor );

#define MACE_STUB_VTABLE_VISIT_MEMBER( r, visitor, elem ) \
  visitor.template operator()<BOOST_TYPEOF(&vtable_type::elem),&vtable_type::elem>( BOOST_PP_STRINGIZE(elem) );

// example of how to convert enumerate any BOOST_PP_SEQ, including BOOST_PP_SEQ_NIL
#define MACE_STUB_SEQ_ENUM(X) \
BOOST_PP_LIST_ENUM( \
  BOOST_PP_LIST_REST( \
    BOOST_PP_TUPLE_TO_LIST( BOOST_PP_INC(BOOST_PP_SEQ_SIZE(X)), BOOST_PP_SEQ_TO_TUPLE( \
    BOOST_PP_SEQ_TRANSFORM( MACE_STUB_VTABLE_PUBLIC_BASE, InterfaceDelegate, BOOST_PP_SEQ_PUSH_FRONT(X,(null)) ) ) )  \
  ) \
)

#endif  // not DOXYGEN

/**
 *  @def MACE_STUB_DERIVED(NAME,INHERITS,MEMBERS)
 *  
 *  @note Must be used at global scope.
 *
 *  @param NAME a fully qualified class name.
 *  @param INHERITS a sequence of fully qualified base classes that NAME inherits from ie: (method1)(method2)(method3)...
 *  @param MEMBERS a sequence of member methods on NAME ie: (method1)(method2)(method3)...
 */
#define MACE_STUB_DERIVED( NAME, INHERITS, MEMBERS ) \
namespace mace { namespace stub { \
template<typename InterfaceDelegate > \
struct vtable<NAME,InterfaceDelegate> : BOOST_PP_SEQ_FOR_EACH( MACE_STUB_VTABLE_PUBLIC_BASE, InterfaceDelegate, INHERITS ) private vtable_base<NAME> { \
    typedef NAME interface_type; \
    typedef InterfaceDelegate delegate_type; \
    BOOST_PP_SEQ_FOR_EACH( MACE_STUB_VTABLE_DEFINE_MEMBER, NAME, MEMBERS ) \
}; \
template<typename InterfaceDelegate> struct vtable_reflector<NAME,InterfaceDelegate> { \
    typedef NAME interface_type; \
    template<typename Visitor> \
    static void visit( const Visitor& visitor ) { \
        typedef mace::stub::vtable<NAME,InterfaceDelegate> vtable_type; \
        BOOST_PP_SEQ_FOR_EACH( MACE_STUB_VTABLE_VISIT_BASE, visitor, INHERITS ) \
        BOOST_PP_SEQ_FOR_EACH( MACE_STUB_VTABLE_VISIT_MEMBER, visitor, MEMBERS ) \
    } \
};\
\
} } 

/**
 *  @def MACE_STUB( NAME, MEMBERS )
 *
 *  @note Must be used at global scope.
 *
 *  @param NAME a fully qualified class name.
 *  @param MEMBERS a sequence of member methods on NAME ie: (method1)(method2)(method3)...
 */
#define MACE_STUB( NAME, MEMBERS ) \
    MACE_STUB_DERIVED( NAME, BOOST_PP_SEQ_NIL, MEMBERS )

#endif
