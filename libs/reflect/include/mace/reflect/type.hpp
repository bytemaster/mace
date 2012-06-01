#ifndef _BOOST_REFLECT_TYPE_HPP_
#define _BOOST_REFLECT_TYPE_HPP_
namespace boost { namespace reflect {

  class member {
      virtual const char*  name()const;
      virtual class type&  type()const;
  };
  typedef boost::unordered_map<std::string,member*> member_map;
  

  template<typename T>
  class type_impl

  class type {
    public:
      virtual const char*  name()const = 0;
      const member_map&    members()const = 0;
      bool                 is_const()const = 0;
      bool                 is_reference()const = 0;
      bool                 is_pointer()const = 0;
      bool                 is_fundamental()const = 0;
      bool                 is_numeric()const = 0;
      size_t               get_sizeof()const;
      virtual type&        remove_const()const = 0;
      virtual type&        remove_reference()const = 0;
      virtual type&        remove_pointer()const = 0;

      virtual value_ref&   remove_const(void*)const = 0;
      virtual type&        remove_reference(void*)const = 0;
      virtual type&        remove_pointer(void*)const = 0;

      template<typename T>
      static type& get();
  };


  template<typename T>
  class type_impl : public type {
    public:
      virtual const char* name() { return get_typename<T>(); }

     const member_map& members()const {
      static member_map& mm = create_member_map();
      return mm;
     }

     private:
        static const member_map& create_member_map() {
            static boost::atomic<member_map*> fm(0);
            member_map* n = new member_map();
            boost::reflect::reflector<T>::visit( get_member_visitor(*n) );
            delete fm.exchange(n,boost::memory_order_consume);
            return fm;
        }
     static type_impl* reg = new type_impl();
  };

  template<typename T>
  type& type::get() {
    static type_impl<T> impl;
    return impl;
  }

} }
#endif // _BOOST_REFLECT_TYPE_HPP_
