#ifndef _BOOST_STUB_ANY_HPP_
#define _BOOST_STUB_ANY_HPP_
#include<utility>
#include <string.h>
#include <iostream>
#include <assert.h>
#include <typeinfo>

namespace mace { namespace stub {

  enum interface_kind {
    abstract_interface,
    forward_interface
  };

  template<template<interface_kind,typename> class Interface>
  class any;

  namespace detail {
    template<template<interface_kind,typename> class Interface>
    class abstract_holder : virtual public Interface<abstract_interface,void> {
      public:
        virtual abstract_holder*  clone_holder_helper(char*)const = 0;
        virtual abstract_holder*  move_holder_helper(char*) = 0;
    };
    
    template<template<interface_kind,typename> class Interface, typename T>
    class holder : public Interface<forward_interface,T>, virtual public abstract_holder<Interface> {
      public:
        template<typename V>
        holder( V&& v )
        :Interface<forward_interface,T>::virtual_storage_type(std::forward<V>(v)),
         Interface<forward_interface,T>( ){}

        holder( const holder& v ) 
        :Interface<forward_interface,T>::virtual_storage_type( static_cast<const typename Interface<forward_interface,T>::virtual_storage_type&>(v) ),
         Interface<forward_interface,T>(v) {}

        holder( typename Interface<forward_interface,T>::virtual_storage_type&& v )
        :Interface<forward_interface,T>::virtual_storage_type(std::move(v)),
         Interface<forward_interface,T>() {}

        holder( holder& v ) 
        :Interface<forward_interface,T>::virtual_storage_type(v.val) {}
        holder(){}

      private:
        template<template<interface_kind,typename> class I>
        friend class any;
        abstract_holder<Interface>* clone_holder_helper( char* p )const { return new (p) holder(*this); }
        abstract_holder<Interface>* move_holder_helper( char* p )       { return new (p) holder(std::move(this->val)); }
    };

    // store by value on the heap
    template<typename T>
    struct any_store {
      any_store( T&& _v):v(new T(std::move(_v))){}
      any_store( const T& _v):v(new T(_v)){}
      any_store( T& _v):v(new T(_v)){}
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
      any_store( T* _v):v(_v){}
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
    
    /**
     *  Implements the store as a polymorphic holder
     */
    template<template<interface_kind,typename> class Interface>
    struct any_store< any<Interface>* >  {
      protected:
        typedef detail::abstract_holder<Interface> abstract_interface;
        const abstract_interface* const to_T()const { return reinterpret_cast<const abstract_interface*>( &impl_place[2*sizeof(void*)] ); }
        abstract_interface* to_T()      {  return reinterpret_cast<abstract_interface*>( &impl_place[2*sizeof(void*)] ); }

        char impl_place[sizeof(detail::holder<Interface,abstract_holder<Interface>*>)];
      public:
        template<typename P>
        any_store(P&& v ) {
           new (&impl_place[0]) detail::holder<Interface,typename std::remove_reference<P>::type>( std::forward<P>(v) );
        }
        any_store( const any_store& c ){
           c.to_T()->clone_holder_helper( (char*)impl_place );
        }
        any_store( any_store& c ){
           c.to_T()->clone_holder_helper( (char*)impl_place );
        }
        any_store( any_store&& c ) {
          c.to_T()->move_holder_helper( &impl_place[0] );
        }
        ~any_store() {
           abstract_interface* t = to_T();
           t->~abstract_interface();
        }
        any_store& operator=( const any_store& c ) {
           if( &c != this ) {
             to_T()->~abstract_interface();
        	   c.to_T()->clone_holder_helper((char*)impl_place);
           }
           return *this;
        }
        template<typename T>
        any_store& operator=( const T& c ) {
           if( &c != this ) {
             to_T()->~T();
             new (&impl_place[0]) detail::holder<Interface,typename std::remove_reference<T>::type>( std::forward<T>(c) );
           }
           return *this;
        }
        template<typename T>
        any_store& operator=( T&& c ) {
           to_T()->~T();
           new (&impl_place[0]) detail::holder<Interface,typename std::remove_reference<T>::type>( std::forward<T>(c) );
           return *this;
        }

        any_store& operator=( any_store&& c ) {
           char tmp[sizeof(impl_place)];
           to_T()->move_holder_helper(tmp);
           c.to_T()->move_holder_helper(impl_place);
           reinterpret_cast<abstract_interface*>(&tmp[2*sizeof(void*)])->move_holder_helper(c.impl_place);
           return *this;
        }


        abstract_interface* operator->()             { return to_T(); }
        const abstract_interface* operator->()const  { return to_T(); }
       
        abstract_interface& operator*(){ return *to_T(); }
        const abstract_interface& operator*()const{ return *to_T(); }
    };
  }
 
  

  template<typename T>
  struct any_store {
    typedef any_store<T> virtual_storage_type;
    template<typename V>
    any_store( V&& v ):val(std::forward<V>(v)){}
    any_store( const any_store& v ):val(v.val){}
    any_store( any_store& v ):val(v.val){}
    any_store( any_store&& v ):val(std::move(v.val)){}

    any_store():val(T()){}

    template<typename Type, typename AnyType>
    friend Type& any_cast( AnyType& at );
    template<typename Type, typename AnyType>
    friend Type* any_cast( AnyType* at );

    virtual const std::type_info&  __get_type_id()const  { return typeid(T); } 
    virtual const void* const      __get_void_ptr()const { return &*val;     }
    virtual void*                  __get_void_ptr()      { return &*val;     }

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
  class any : public detail::holder<Interface, any<Interface>* > {
    private:
      typedef detail::abstract_holder<Interface> abstract_holder;

    public:
      template<typename T>
      any( T&& v )
      :any_store<any*>( std::forward<T>(v) ){}
      
      any( any& v )
      :any_store<any*>( static_cast<any_store<any*>&>(v) ){}
      
      any( const any& v )
      :any_store<any*>( static_cast<const any_store<any*>&>(v) ) {}

      any( any&& v )
      :any_store<any*>( static_cast<any_store<any*>&&>(v))  {}
      
      template<typename T>
      any& operator=( T&& v ) {
        static_cast<any_store<any<Interface>*>&>(*this) = std::move(v);
        return *this;
      }
      
      any& operator=(const any& v) {
        if( this != & v ) {
           static_cast<any_store<any<Interface>*>&>(*this) =  static_cast<const any_store<any<Interface>*>&>(v);
        }
        return *this;
      }
      any& operator=( any&& v ) {
        static_cast<any_store<any<Interface>*>&>(*this) = static_cast<any_store<any<Interface>*>&&>(v);
        return *this;
      }

    private: 
      any(); // not implemented, should not be used.
  };

  template<typename T, typename AnyType>
  T& any_cast( AnyType& at ) {
     if( at.__get_type_id()  == typeid(T) ) {
       return *static_cast<T*>( at.___void_ptr() );
     }
     throw std::bad_cast();
  }
  template<typename T, typename AnyType>
  T* any_cast( AnyType* at ) {
     if( at && at->__get_type_id()  == typeid(T) ) {
       return *static_cast<T*>( at.___void_ptr() );
     }
     return 0;
  }
  

  template<template<interface_kind,typename> class Interface>
  std::ostream& operator<<( std::ostream& os, const any<Interface>& a ) {
    return a.operator<<(os);
  }

} } 



#endif
