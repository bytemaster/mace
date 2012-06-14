#ifndef _BOOST_STUB_ANY_HPP_
#define _BOOST_STUB_ANY_HPP_
#include<utility>
#include <string.h>
#include <iostream>

namespace mace { namespace stub {

  enum interface_kind {
    abstract_interface,
    forward_interface
  };

  namespace detail {
    template<template<interface_kind,typename> class Interface>
    class abstract_holder : virtual public Interface<abstract_interface,void> {
      public:
        virtual abstract_holder*  clone_holder_helper(char*) = 0;
        virtual abstract_holder*  move_holder_helper(char*) = 0;
    };
    
    template<template<interface_kind,typename> class Interface, typename T>
    class holder : public Interface<forward_interface,T>, virtual public abstract_holder<Interface> {
      public:
        template<typename V>
        holder( V&& v )
        :Interface<forward_interface,T>::virtual_storage_type(std::forward<V>(v)),
         Interface<forward_interface,T>( )
         {
          std::cerr<<"                          create holder at ";
          std::cerr<<(void*)this<<std::endl;
        };
        ~holder() {}

        holder( const holder& v ) 
        :Interface<forward_interface,T>::virtual_storage_type(v),
         Interface<forward_interface,T>(v) {
          std::cerr<<"\t\t                                             copy holder "<<this<<std::endl;
        }
        holder( typename Interface<forward_interface,T>::virtual_storage_type&& v )
        :Interface<forward_interface,T>::virtual_storage_type(std::move(v)),
         Interface<forward_interface,T>() {
          std::cerr<<"\t\t                                             copy holder "<<this<<std::endl;
        }
        holder( holder& v ) 
        :Interface<forward_interface,T>::virtual_storage_type(v.val)
        {
          std::cerr<<"\t\t                                             copy holder "<<this<<std::endl;
        }
        holder(){ 
          std::cerr<<"default holder called..\n";
        }

      private:
        template<template<interface_kind,typename> class I>
        friend class any;
        abstract_holder<Interface>* clone_holder_helper( char* p ) { std::cerr<<"-----------IN PLACE CLONE___----\n"; return new (p) holder(*this); }
        abstract_holder<Interface>* move_holder_helper( char* p )  { std::cerr<<"-----------IN PLACE CLONE___----\n"; return new (p) holder(std::move(this->val)); }
    };

    // store by value on the heap
    template<typename T>
    struct any_store {
      any_store( T&& _v):v(new T(std::move(_v))){ std::cerr<<"store move T"<<this<<"\n"; }
      any_store( const T& _v):v(new T(_v)){ std::cerr<<"store const & T"<<this<<"\n"; }
      any_store( T& _v):v(new T(_v)){ std::cerr<<"store T& "<<this<<"\n"; }
      any_store( const any_store& a )
      :v(new T(*a.v)){}

      any_store( any_store&& a )
      :v(a.v){ a.v=0; }

      ~any_store() { delete v; }
      any_store():v(0){}

      any_store& operator=( any_store&& other ) {
        std::swap( v, other.v );
        return *this;
      }

      any_store& operator=( const any_store& other ) { 
        if( this != &other ) {
          delete other;
          v = other.v ? new T(*other.v) : 0;
        }
        return *this;
      }
    
      T* operator->()             { return v; }
      const T* operator->()const  { return v; }
      T& operator*(){ return *v; }
      const T& operator*()const{ return *v; }
     // private:
      T* v;
    };
    

    // pointers and ref store directly
    template<typename T>
    struct any_store<T*> {
      any_store( T* _v):v(_v){
        std::cerr<<"storing ptr "<<v<<std::endl;
      }
      any_store( const any_store& a )
      :v(a.v){}
    
      T* operator->()             { return v; }
      const T* operator->()const  { return v; }
      
      T& operator*(){ return *v; }
      const T& operator*()const{ return *v; }
    
      //private:
      T* v;
    };
    template<typename T>
    struct any_store<T**> {
      any_store( T** _v):v(_v){}
      any_store( const any_store& a )
      :v(a.v){}
    
      T* operator->()             { return *v; }
      const T* operator->()const  { return *v; }

      T& operator*(){ return **v; }
      const T& operator*()const{ return **v; }
    
      //private:
      T** v;
    };

    template<typename T>
    struct any_store<T&> {
      any_store( T& _v):v(_v){}
      any_store( const any_store& a )
      :v(a.v){}
    
      T* operator->()             { return &v; }
      const T* operator->()const  { return &v; }

      T& operator*(){ return v; }
      const T& operator*()const{ return v; }
    
     // private:
      T& v;
    };
  }

  template<typename T>
  struct any_store {
    typedef any_store<T> virtual_storage_type;
    template<typename V>
    any_store( V&& v ):val(std::forward<V>(v)){}

    any_store( const any_store& v ):val(v.val){}
    any_store( any_store&& v ):val(std::move(val)){}

    any_store():val(T()){ std::cerr<<"default any store: "<<this<<std::endl;}
    // normalizes calling convention to pointer semantics
    // determines where to store the data
    detail::any_store<T> val;
  };


  /**
   *  Provides non-intrusive dynamic polymorphism over InterfaceType
   *
   *  @tparam InterfaceSpec - specifies which interface spec to use.  An interface spec
   *            defines the methods supported by the any.
   *
   *  This type is specialized by the MACE_STUB_ANY() macro which enable the 
   *  use of InterfaceType.
   */
  template<template<interface_kind,typename> class Interface>
  class any : public detail::holder<Interface,detail::abstract_holder<Interface>**> {
    private:
      typedef detail::abstract_holder<Interface>         		     abstract_holder;
      typedef detail::holder<Interface,detail::abstract_holder<Interface>**> base_type;

      friend abstract_holder* get_holder( any* a ) {
        return static_cast<abstract_holder*>((void*)a->impl);
      }

      friend const abstract_holder* const get_holder( const any* a ) {
        return static_cast<const abstract_holder* const>((void*)a->impl);
      }
      char impl_place[sizeof(detail::holder<Interface,abstract_holder*>)];
      abstract_holder* impl;

    public:
      template<typename T>
      any( T&& v )
      :any_store<abstract_holder**>(&impl)
      {
        static_assert( sizeof(impl_place) >= sizeof(detail::holder<Interface,T>),"hi" );
        impl = new ((char*)impl_place) detail::holder<Interface,typename std::remove_reference<T>::type>(std::forward<T>(v));
      }
      
      any( any& v )
      :any_store<abstract_holder**>(&impl) {
        impl = v.impl->clone_holder_helper((char*)impl_place);
      }
      
      any( const any& v )
      :any_store<abstract_holder**>(&impl) {
        impl = v.impl->clone_holder_helper((char*)impl_place);
      }

      any( any&& v )
      :any_store<abstract_holder**>(&impl)/*,base_type( &impl )*/ {
        impl = v.impl->move_holder_helper((char*)impl_place);
        v.impl = 0;
      }

      ~any(){
        if(impl) impl->~abstract_holder();;
      }
      
      template<typename T>
      any& operator=( T&& v ) {
        return *this;
      }
      
      any& operator=(const any& v) {
        if( this != & v ) {
        	if(impl) impl->~abstract_holder();;
        	impl = v.impl->clone_holder_helper((char*)impl_place);
        }
        return *this;
      }
      any& operator=( any&& v ) {
        if(impl) impl->~abstract_holder();
        impl = v.impl->move_holder_helper((char*)impl_place);
        v.impl = 0;
        return *this;
      }
  };

  template<template<interface_kind,typename> class Interface>
  std::ostream& operator<<( std::ostream& os, const any<Interface>& a ) {
    return a.val->operator<<(os);
  }

} } 



#endif
