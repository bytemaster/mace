#ifndef _MACE_CMT_ACTOR_HPP_
#define _MACE_CMT_ACTOR_HPP_
#include <mace/cmt/thread.hpp>
#include <mace/stub/ptr.hpp>
#include <mace/cmt/actor_interface.hpp>

namespace mace { namespace cmt {

  /**
   *  An actor is assigned to a specific thread, and if a method is called on the actor
   *  from another thread then it is posted asynchronously to the proper thread.
   *
   *  All methods return future<R>.
   *
   */
  template<typename InterfaceType>
  class actor : public mace::stub::ptr<InterfaceType, mace::cmt::actor_interface>, public detail::actor_base {
    public:
      typedef boost::shared_ptr<actor>   ptr;

      actor(mace::cmt::thread* t = mace::cmt::thread::current() )
      :actor_base(t){
      }
      template<typename T>
      actor( T* v, mace::cmt::thread* t = mace::cmt::thread::current() )  
      :actor_base(t) {
        this->m_ptr = boost::make_shared<boost::any>(v);
        cmt::actor_interface::set_vtable(*this->m_vtable,*v,this);
      }
      template<typename T>
      actor( const boost::shared_ptr<T>& v,mace::cmt::thread* t = mace::cmt::thread::current() ) 
      :actor_base(t) {
        this->m_ptr = boost::make_shared<boost::any>();
        *(this->m_ptr) = v;
        cmt::actor_interface::set_vtable(*this->m_vtable,*v,this);
      }
      template<typename OtherInterface,typename OtherDelegate>
      actor( const mace::stub::ptr<OtherInterface,OtherDelegate>& v,
             mace::cmt::thread* t = mace::cmt::thread::current() ) 
      :actor_base(t) {
        this->m_ptr = boost::make_shared<boost::any>();
        *(this->m_ptr) = v;
        /// @todo fix this
        //cmt::actor_interface::set_vtable( *this->m_vtable, 
        //                  *boost::any_cast<mace::stub::ptr<OtherInterface,OtherDelegate>&>(this->m_ptr), this );
      }
  };

} } // namespace mace::cmt

#endif

