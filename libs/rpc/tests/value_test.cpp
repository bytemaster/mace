#define BOOST_TEST_MODULE MACE_RPC_VALUE
#include <boost/test/unit_test.hpp>
#include <mace/rpc/value.hpp>
#include <mace/rpc/value_io.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <mace/rpc/json/io.hpp>

using namespace mace::rpc;

struct test_s {
  test_s():a("aaa"),b(222),c(333){}
  std::string a;
  int         b;
  double      c;

  bool operator == ( const test_s& s )const {
    return s.a == a && s.b == b && s.c == c;
  }
};
struct test_sub {
  test_sub():d("ddd"){}
  std::string d;
  test_s      sub;

  bool operator == ( const test_sub& s )const {
    return s.sub == sub && d == s.d;
  }
};

MACE_REFLECT( test_s, (a)(b)(c) )
MACE_REFLECT( test_sub, (d)(sub) )

BOOST_AUTO_TEST_CASE( null_test ) {
  mace::rpc::value v;
  BOOST_REQUIRE( v.type() == std::string("null" ) );
}
BOOST_AUTO_TEST_CASE( int_test ) {
  mace::rpc::value v = 5;
  BOOST_REQUIRE( v.type() == std::string("int32_t" ) );
}
BOOST_AUTO_TEST_CASE( string_test ) {
  mace::rpc::value v = std::string("Hello World");
  BOOST_REQUIRE( v.type() == std::string("string" ) );
  mace::rpc::value v2 = std::string("Hello World");
  BOOST_REQUIRE( v2.type() == std::string("string" ) );
  mace::rpc::value v3;
  v3 = std::string("Hello World");
  BOOST_REQUIRE( v3.type() == std::string("string" ) );
}
BOOST_AUTO_TEST_CASE( int_cast ) {
  mace::rpc::value v = 5;
  BOOST_REQUIRE( v.type() == std::string("int32_t" ) );
  int64_t c = mace::rpc::value_cast<int64_t>(v);
  BOOST_CHECK( c == 5 );
  double d = mace::rpc::value_cast<double>(v);
  BOOST_CHECK( d == 5 );
  float f = mace::rpc::value_cast<float>(v);
  BOOST_CHECK( f == 5 );
  std::string s = mace::rpc::value_cast<std::string>(v);
  BOOST_CHECK( s == "5" );
}
BOOST_AUTO_TEST_CASE( double_cast ) {
  value v = 5.5;
  auto f = mace::rpc::value_cast<int>(v);
  BOOST_CHECK( f == 5 );
}
BOOST_AUTO_TEST_CASE( overflow_cast ) {
  int64_t in = 99999999;
  value v = in;
  BOOST_CHECK_THROW( mace::rpc::value_cast<int8_t>(v), std::bad_cast );
}


BOOST_AUTO_TEST_CASE( value_vector ) {
  std::vector<int> vec;
  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);
  mace::rpc::value v = value(vec);
  BOOST_REQUIRE( v.type() == std::string("array" ) );
  BOOST_REQUIRE( v.size() == 3 );
  BOOST_REQUIRE( value_cast<int>(v[0]) == 1 );
  BOOST_REQUIRE( value_cast<int>(v[1]) == 2 );
  BOOST_REQUIRE( value_cast<int>(v[2]) == 3 );
}
BOOST_AUTO_TEST_CASE( value_struct ) {
  test_sub ts;
  ts.sub.a = "__AA";
  value v = value(ts);
  BOOST_REQUIRE( v.type() == std::string("object" ) );
  BOOST_REQUIRE( value_cast<std::string>(v["d"]) == "ddd" );
  BOOST_REQUIRE( value_cast<std::string>(v["sub"]["a"]) == "__AA" );

  test_s o;
  unpack( v["sub"], o );
  BOOST_REQUIRE( o.a == "__AA" );
  test_sub s;
  unpack( v, s );
  BOOST_REQUIRE( s.sub.a == "__AA" );
  BOOST_REQUIRE( value_cast<test_s>(v["sub"]).a == "__AA" );
}

BOOST_AUTO_TEST_CASE( value_seq ) {
  value v = value(boost::fusion::make_vector( 5, "Hello World", 6.6 ));
  BOOST_REQUIRE( value_cast<int>(v[0]) == 5 );
  BOOST_REQUIRE( value_cast<std::string>(v[1]) == "Hello World" );
  BOOST_REQUIRE( value_cast<double>(v[2]) ==  6.6 );
  auto f = boost::fusion::make_vector( 5, "Hello World", 6.6 );
  value v2 = value(f);
  BOOST_REQUIRE( value_cast<int>(v2[0]) == 5 );
  BOOST_REQUIRE( value_cast<std::string>(v2[1]) == "Hello World" );
  BOOST_REQUIRE( value_cast<double>(v2[2]) ==  6.6 );

  typedef boost::fusion::vector<int,std::string,double> seq;
  seq out = value_cast<seq>(v2);
  BOOST_REQUIRE( boost::fusion::at_c<0>(out) == boost::fusion::at_c<0>(f)  );
  BOOST_REQUIRE( boost::fusion::at_c<1>(out) == boost::fusion::at_c<1>(f)  );
  BOOST_REQUIRE( boost::fusion::at_c<2>(out) == boost::fusion::at_c<2>(f)  );
}

BOOST_AUTO_TEST_CASE( value_from_json ) {
  test_sub s;
  s.d = "world";
  s.sub.a = "moo";
  s.sub.b = 44;
  s.sub.c = 55.5;
  auto js = mace::rpc::json::io::pack(s);
  json::error_collector ec;
  value v = json::to_value(std::move(js),ec);
  auto out = value_cast<test_sub>(v);
  BOOST_REQUIRE(out.d == s.d);
  BOOST_REQUIRE(out.sub.a == s.sub.a);
  BOOST_REQUIRE(out.sub.b == s.sub.b);
  BOOST_REQUIRE(out.sub.c == s.sub.c);
}

BOOST_AUTO_TEST_CASE( value_new_index ) {
  value v;
  v["area"] = 5.5;
  BOOST_REQUIRE( value_cast<double>(v["area"]) == 5.5 );
}

BOOST_AUTO_TEST_CASE( parse_test ) {
  test_sub ts;
  ts.sub.a = "__AA";
  std::string j = mace::rpc::json::to_string(ts);
  BOOST_REQUIRE( ts == mace::rpc::json::from_string<test_sub>(j) );
}
BOOST_AUTO_TEST_CASE( parse_pretty_test ) {
  test_sub ts;
  ts.sub.a = "__AA";
  ts.sub.b = 5;;
  ts.sub.c = 6;;
  ts.d = "7";
  std::string j = mace::rpc::json::to_pretty_string(ts);
  std::cout<<j<<std::endl;
  auto f = mace::rpc::json::from_string<test_sub>(j);
  BOOST_REQUIRE( ts == f );
  std::string j2 = mace::rpc::json::to_pretty_string(f);
  BOOST_REQUIRE( j2 == j );
}
BOOST_AUTO_TEST_CASE( parse_bug ) {
  std::string j = "{\"uuid\":\"1340315652715724000nanosecondssinceJan1,1970\",\"name\":\"test\",\"description\":\"\""
                  ",\"components\":{\"root.areacalc\":{\"uuid\":\"10234\",\"type\":\"area\",\"path\":\"\"}},\"structs\":{}}";
  mace::rpc::json::error_collector ec;    
  value v = json::to_value( std::vector<char>( j.begin(),j.end()), ec );
  auto o = json::to_string(v);
  slog( "%1%\n%2%", o, j );
  BOOST_REQUIRE( o == j );
}

