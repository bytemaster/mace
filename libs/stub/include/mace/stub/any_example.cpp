//#include <mace/stub/any.hpp>
#include <string>
#include <iostream>
#include "any.hpp"

using namespace mace::stub;

// this interface is specialized below
template<interface_kind InterfaceKind=forward_interface, typename T=void>
struct  my_interface{
    virtual ~my_interface() {}
    virtual int add( int ) = 0;
    virtual int add( double, std::string )  = 0;
    virtual int sub( int ) = 0;
    virtual my_interface& operator+=( int x ) = 0;
    virtual std::ostream& operator<<( std::ostream& os )const =0;
};

// T could be a reference or pointer, any_store handles those cases...
template<typename T>
struct my_interface<forward_interface,T> : virtual public any_store<T>,
                                           virtual public my_interface<abstract_interface> {

    // now define how your interface wraps T with virtual methods....
    // these methods must implement those defined for my_interface<abstract_interface,void>
    virtual int add( int i)                   { return this->val->add(i);     }
    virtual int add( double d, std::string s) { return this->val->add( d, s); }
    virtual int sub( int i)                   { return this->val->sub( i );   }
    virtual std::ostream& operator<<( std::ostream& os )const { return this->val->operator<<(os); }
    virtual my_interface<abstract_interface>& operator+=( int x ) { *(this->val) +=(x); return *this; }
};

static int c = 0;
struct test {
  int v;
  test(int _v ):v(_v) {
    ++c;
    std::cerr<<c<<"                            test: "<<this<<std::endl;
  }
  ~test() {
    --c;
    std::cerr<<c<<"                            ~test: "<<this<<std::endl;
  }
  test( const test& t ):v(t.v){
    ++c;
    std::cerr<<"                                copy from "<<&t<<" to "<<this<<std::endl;
  }
  test( test&& t ):v(t.v) {
    ++c;
    std::cerr<<"                                 move from "<<&t<<" to "<<this<<std::endl;
  }
    int add( int i )  { std::cerr<<"###############  add this "<<this<<"                                               v: "<<v<<std::endl; return i + 5; }
    int add( double, std::string ) { return 6; }
    int sub( int i ) { return i - 5; }

  test& operator+=(int x) { v += x; return *this; }

std::ostream& operator<<( std::ostream& os )const { return os << v; }
 

  test(){ ++c; std::cerr<<"EXIT\n"; exit(1); }
};

std::ostream& operator<<(std::ostream& o, const test& t ) {
	return o << t.v;
}

int main( int argc, char** argv ) {
  {
std::cerr<<" ------------INIT REF TEST--------------\n";
  test t(1);
  any<my_interface> v = t;

  std::cerr<<v.add(int(5))<<" == 10\n";
  std::cerr<<v.add(double(5.5), "hello")<<" == 6\n";
  std::cerr<<v.sub(int(5))<<" == 0\n";

std::cerr<<" ------------COPY TEST--------------\n";
  any<my_interface> v2 = v;
  std::cerr<<v2.add(88);

std::cerr<<" ----------- CREATE FROM TEMP ------\n";
  any<my_interface> mt = test(2);
std::cerr<<" ------------MOVE TEST--------------\n";
  any<my_interface> v3  = std::move(mt);
  v3.add(99);
std::cerr<<" ------------POINTER TEST--------------\n";
  any<my_interface> v4 = &t;
  std::cerr<<v4.add(1000)<<std::endl;
  any<my_interface> v5 = v4;
  t.v = 99;
  std::cerr<<v5.add(1000)<<std::endl;
  
std::cerr<<" ------------ADD TEST--------------\n";
  v5 += 30000;
//  std::cerr<<v5.add(1000)<<std::endl;
  std::cerr<<v5<<v4<<v3;
  } 

  std::cerr<<"SIZE: "<<sizeof(any<my_interface>)<<std::endl;
  std::cerr<<"tSIZE: "<<sizeof(test)<<std::endl;
  std::cerr<<"tSIZE: "<<sizeof(detail::holder<my_interface,test>)<<std::endl;

  std::cerr<<"C: "<<c<<std::endl;
  return 0;
}

