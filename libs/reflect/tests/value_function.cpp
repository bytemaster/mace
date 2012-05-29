#include <boost/cmt/log/log.hpp>
#include <boost/exception/all.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <mace/reflect/value.hpp>
#include <utility>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/function_types/result_type.hpp>

using namespace mace::reflect;

struct test_struct {
  std::string test_func( int x, double y ) { return boost::lexical_cast<std::string>(x*y); };
  std::string boo;
  int boo2;
};


using namespace boost;
using namespace boost::mpl;
using namespace boost::function_types;
      class test_visitor {
        public:
          template<typename T,  T v >
          inline void operator()( const char* name )const {
            std::cerr<< name <<" " <<mace::reflect::get_typename<typename boost::function_types::result_type<T>::type>() <<std::endl;
            typedef typename boost::function_types::result_type<T>::type result_type;
            BOOST_STATIC_ASSERT( (boost::is_same<result_type,std::string>::value) );
            BOOST_STATIC_ASSERT( (boost::is_same<typename mpl::at_c<components<T,identity<_> >,1>::type,test_struct>::value) );

//            BOOST_STATIC_ASSERT( (boost::is_same< boost::mpl::at<boost::function_types::components<T>, mpl::int_<0> >,std::string>::value) );
            //typedef typename boost::mpl::at< boost::function_types::components<T,boost::mpl::identity<boost::mpl::_> >,boost::mpl::int_<5> >::type class_type;
            typedef typename boost::mpl::at< boost::function_types::components<T,boost::add_pointer<boost::mpl::_> >, boost::mpl::int_<1> >::type class_type;
          }
       };

//BOOST_REFLECT( test_struct, (test_func) );

int main( int argc, char** argv ){
  test_visitor v;

  //v.operator()<std::string,test_struct,int,double,&test_struct::test_func>("name");
  v.operator()<BOOST_TYPEOF(&test_struct::test_func), &test_struct::test_func>("name");
  //v.operator()<BOOST_TYPEOF(&test_struct::boo), &test_struct::boo>("boo");
  //v.operator()<BOOST_TYPEOF(&test_struct::boo2), &test_struct::boo2>("boo2");
//  value v(test_struct());
//  BOOST_ASSERT( v["test_func"]( 3, 5.5 ).as<std::string>() == boost::lexical_cast<std::string>(3*5.5) );
  return 0;
}
