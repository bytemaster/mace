/**
 *  @file mace/cmt/task.hpp
 *
 *  @todo move into detail folder, this is not part of the public api.
 */
#ifndef MACE_CMT_TASK_HPP
#define MACE_CMT_TASK_HPP
#include <boost/enable_shared_from_this.hpp>
#include <mace/cmt/error.hpp>
#include <mace/cmt/retainable.hpp>
#include <mace/cmt/future.hpp>
#include <mace/cmt/spin_lock.hpp>
#include <boost/function.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <mace/cmt/log/log.hpp>
#include <mace/cmt/priority.hpp>

namespace mace { namespace cmt {
  using namespace boost::chrono;

   struct context;

  /**
   *  @todo 
   *    Eliminate the need for virtual functions as the number of potential
   *    functors involved would result in RTTI code bloat explosion.  The cost
   *    would be implementing a 'manual' vtable for run() and destructor()
   *
   *  @todo
   *    Convert a task into a promise!  This would eliminate another alloc per
   *    async operation as only the task would require an alloc.  The downside 
   *    is that extra memory would be kept around until no-one needed the 
   *    task any more.
   *
   *  @todo track the parent/child relationship with all async operations so that
   *    when the parent is canceled, all child tasks get canceled as well.
   *
   *  @todo Eliminate vtask as redundant once all tasks are promises and
   *    there is no 'extra' overhead associated with returning a promise.
   *
   */
  class task {
    public:
      typedef task* ptr;
      task(priority p=priority(),const system_clock::time_point& w = system_clock::time_point::min() )
      :when(w),canceled(false),prio(p),next(0),active_context(0),desc(""){
        static int64_t global_task_count=0;
        posted_num = ++global_task_count;
      }
      virtual ~task() { }

      virtual void run() = 0;
      /// implemented in thread.cpp
      void cancel();

      void set_desc( const char*  d ) { desc=d; }
      const char* get_desc()const { return desc; }

    protected:
      void set_active_context( cmt::context* c ) {
        boost::unique_lock<cmt::spin_lock> lock( active_context_lock );
        active_context = c;
      }
      static int64_t    task_num;
      friend class      thread;
      friend class      thread_private;

      system_clock::time_point when;
      bool                     canceled;
      uint64_t                 posted_num;
      priority                 prio;
      task*                    next;
      cmt::context*            active_context;
      mace::cmt::spin_lock     active_context_lock;
      const char*              desc;
  };

  template<typename Functor, typename R = void>
  class rtask : public task {
    public:
      template<typename F>
      rtask( F&& f, const typename promise<R>::ptr& p, const system_clock::time_point& tp, priority prio )
      :task(prio,tp),m_functor(std::forward<F>(f)),m_prom(p){ m_prom->set_task(this); }

      template<typename F>
      rtask( F&& f, const typename promise<R>::ptr& p, priority prio )
      :task(prio),m_functor(std::forward<F>(f)),m_prom(p){ m_prom->set_task(this); }
      ~rtask(){ m_prom->set_task(0); }

      void run() {
        try {
          if( !canceled )
            m_prom->set_value( m_functor() );
          else
            m_prom->set_exception( boost::copy_exception( error::task_canceled() ) );
        } catch( ... ) {
          m_prom->set_exception(boost::current_exception());
        }
      }

      Functor           m_functor;
      typename promise<R>::ptr  m_prom;
  };

  template<typename Functor>
  class rtask<Functor,void> : public task {
    public:
      template<typename F>
      rtask( F&& f, const promise<void>::ptr& p, const system_clock::time_point& tp, priority prio )
      :task(prio,tp),m_functor(std::forward<F>(f)),m_prom(p){ m_prom->set_task(this); }

      template<typename F>
      rtask( F&& f, const  promise<void>::ptr& p, priority prio=priority() )
      :task(prio),m_functor(std::forward<F>(f)),m_prom(p){ m_prom->set_task(this); }
      ~rtask() { m_prom->set_task(0); }

      void run() {
        try {
          if( !canceled ) {
            m_functor();
            m_prom->set_value( void_t() );
          } else
            m_prom->set_exception( boost::copy_exception( error::task_canceled() ) );
        } catch( ... ) {
          m_prom->set_exception(boost::current_exception());
        }
      }
    
      Functor       m_functor;
      promise<void>::ptr  m_prom;
  };

  template<typename Functor>
  class vtask : public task {
    public:
    template<typename F>
    vtask( F&& f, priority prio = priority() )
    :task(prio),m_functor(std::forward<F>(f)){ }
    template<typename F>
    vtask( F&& f, const system_clock::time_point& when, priority prio = priority() )
    :task(prio,when),m_functor(std::forward<F>(f)){ }

    void run() {
      try {
        if( !canceled )
        m_functor();
      } catch( const boost::exception& e ) {
        elog( "%1%", boost::diagnostic_information(e) );
      } catch( const std::exception& e ) {
        elog( "%1%", boost::diagnostic_information(e) );
      } catch( ... ) {
        BOOST_ASSERT(!"unhandled exception");
      }
    }
    Functor           m_functor;
  };

} } // namespace mace::cmt

#endif // MACE_CMT_TASK_HPP
