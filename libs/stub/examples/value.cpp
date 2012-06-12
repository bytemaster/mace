#ifndef _BOOST_VALUE_HPP_
#define _BOOST_VALUE_HPP_
#include<iostream>
#include <utility>
  
template<typename Interface>
class value_base {
  public:
  template<typename T>
  value_base( T&& v ) {
    impl = new holder<typename std::remove_reference<T>::type>(std::forward<T>(v));
  }
  value_base( const value_base& v )
  :impl(0) {
    if( v.impl ) impl = v.impl->clone_holder_helper();
  }
  value_base( value_base&& v )
  :impl(v.impl) { v.impl = 0; }

  value_base& operator=(value_base b) {
    std::swap(b.impl,impl);
    return *this;
  }
  value_base():impl(0){}
  ~value_base(){delete impl;}

  struct holder_base : virtual public Interface {
    virtual holder_base*  clone_holder_helper() = 0;
  };
  template<typename T>
  struct holder : public T, public holder_base {
    holder( T&& v ):T( std::move(std::forward<T>(v)) ){};
    holder_base* clone_holder_helper() { return new holder(*this); }
  };

  template<typename T>
  value_base& operator=( T v ) {
    impl = new holder<T>(std::move(v));
    return *this;
  }

  value_base& operator=( value_base&& b ) {
    std::swap(impl,b.impl);
    return *this;
  }

  holder_base*  impl;
};

template<typename T>
struct value{};

class my_interface {
  public:
    virtual ~my_interface() {};
    virtual int add( int ) = 0;
    virtual int add( double, std::string ) = 0;
    virtual int sub( int ) = 0;
};

template<>
struct value<my_interface> : public value_base<my_interface>  {
  /** boiler plate could be removed by inheriting constructors **/
  value(){}
  template<typename T>
  value( T&& v ) {
    this->impl = new holder<typename std::remove_reference<T>::type>(std::forward<T>(v));
  }
  ~value() { delete impl; impl = 0; } // avoid virtual destrutor on base_value
  value( const value& v ) { 
    if( v.impl ) impl = v.impl->clone_holder_helper();
  }
  value( value&& v ):value_base<my_interface>( static_cast<value_base&&>(std::move(v))){}

  using value_base<my_interface>::operator=;
  
  // this is what the macro would most expand
  template<typename... Args>
  auto add(Args&&... args) -> decltype(  ((my_interface*)0)->add( std::forward<Args>(args)... ) ) {
    return impl->add( std::forward<Args>(args)... );
  }
  // this is what the macro would most expand
  template<typename... Args>
  auto sub(Args&&... args) -> decltype(  ((my_interface*)0)->sub( std::forward<Args>(args)... ) ) {
    return impl->sub( std::forward<Args>(args)... );
  }
};


class test : virtual public my_interface {
    int add( int i )  { return i + 5; }
    int add( double, std::string ) { return 6; }
    int sub( int i ) { return i - 5; }
};

int main( int argc, char** argv ) {
  value<my_interface> v;
  v = test();

  std::cerr<<v.add(int(5))<<" == 10\n";
  std::cerr<<v.add(double(5.5), "hello")<<" == 6\n";
  std::cerr<<v.sub(int(5))<<" == 0\n";
  return 0;
}


#endif 
