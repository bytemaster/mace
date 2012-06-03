/**
 * @file mace/cmt/thread.hpp
 */
#ifndef MACE_CMT_THREAD_HPP
#define MACE_CMT_THREAD_HPP
#include <vector>
#include <mace/cmt/task.hpp>
#include <mace/cmt/retainable.hpp>
#include <boost/chrono.hpp>

namespace mace { 
/**
 *  @brief All types that are part of the MACE Cooperative Multi-Tasking Library
 */
namespace cmt {
   using boost::chrono::microseconds;
   using boost::chrono::system_clock;

   class abstract_thread : public retainable {
    public:
      typedef retainable_ptr<abstract_thread> ptr;

      virtual ~abstract_thread(){};
    protected:
      friend class promise_base;
      virtual void wait( const promise_base::ptr& p, const boost::chrono::microseconds& timeout_us ) = 0;
      virtual void wait( const promise_base::ptr& p, const system_clock::time_point& timeout ) = 0;
      virtual void notify( const promise_base::ptr& p ) = 0;
   };
   priority current_priority();
   inline void usleep( uint64_t us );
   inline void sleep_until( const system_clock::time_point& tp );

  /**
  * @brief manages cooperative scheduling of tasks within a single operating system thread.
  *
  * When ever a task must wait, a new stack is created that continues to process events. These
  * stacks will not be deallocated, but instead stand ready to be re-used the next time a
  * new stack is required.   
  *
  * This means that the total number of stacks in use for a given thread will grow to the
  * maximum number of simultaniously waiting asynchronous operations.  
  *
  * @todo perform 'garbage collection' on idle stacks, perhaps before entering a blocking wait.
  *   Idle stacks would be any stack that is 'ready' and not blocking and would simply take
  *   turns processing new tasks.  
  *
  * These todo items may be ignored so long as you use 'long-lived' threads and
  * keep the maximum number of simultanous 'blocked' tasks to something reasonable.  
  */
  class thread : public abstract_thread {
    public:
    /**
     *  Returns the current thread.
     */
    static thread& current();

    /**
     *  @brief returns the name given by @ref set_name() for this thread
     */
    const char* name()const;

    /**
     *  @brief associates a name with this thread.
     */
    void    set_name( const char* n );

    /**
     *  Creates a new thread with the given name.
     *
     *  @todo this method currently blocks the calling thread while a new
     *    thread is created.  It should return a future<thread*> instead.
     */
    static thread* create( const char* name = ""  );


    /**
     *  Calls function <code>f</code> in this thread and returns a future<T> that can
     *  be used to wait on the result.
     *
     *  @param f the method to be called
     *  @param prio the priority of this method relative to others
     *  @param when determines when this call will happen, as soon as 
     *        possible after <code>when</code>
     *  @param n a debug name to be associated with this task. 
     */
    template<typename Functor>
    auto schedule( Functor&& f, const system_clock::time_point& when, 
                   priority prio = priority(), const char* n= "" ) -> future<decltype(f())> {
         typedef decltype(f()) Result;
         typename promise<Result>::ptr p(new promise<Result>());
         task::ptr tsk( new rtask<Functor,Result>( std::forward<Functor>(f),p,when,std::max(current_priority(),prio),n) );
         async_task(tsk);
         return p;
    }


    /**
     *  Calls function <code>f</code> in this thread and returns a future<T> that can
     *  be used to wait on the result.  
     *
     *  @param f the operation to perform
     *  @param prio the priority relative to other tasks
     *  @param n a debut name to associate with this task.
     */
    template<typename Functor>
    auto async( Functor&& f, priority prio = priority(), const char* n= "" ) -> future<decltype(f())> {
       typedef decltype(f()) Result;
       typename promise<Result>::ptr p(new promise<Result>());
       task::ptr tsk( new rtask<Functor,Result>( std::forward<Functor>(f),p,std::max(current_priority(),prio),n) );
       async_task(tsk);
       return p;
    }

    /**
     *  Performs <code>f()</code> asynchronously at the specified time
     *  without returning a future.  This is slightly more effecient as there is no 
     *  need to allocate a promise object.
     *
     *  @param f the method to be called
     *  @param prio the priority of this method relative to others
     *  @param when - determines when this call will happen, as soon as 
     *        possible after <code>when</code>
     *  @param n a debug name to be associated with this task. 
     */
    template<typename Functor>
    void post( Functor&& f, const system_clock::time_point& when, 
                        priority prio = priority(), const char* n= "" ) {
      task::ptr tsk( new vtask<Functor>(std::forward<Functor>(f),when,std::max(current_priority(),prio),n) );
      async_task(tsk);
    }

    /**
     *  Calls function <code>f</code> in this thread. Returns immediately.
     *
     *  @param p - the priority of a task.
     *  @param t - the task to be run.
     */
    template<typename Functor>
    void post( Functor&& f, priority p = priority() ) {
      task::ptr tsk( new vtask<Functor>(std::forward<Functor>(f),std::max(current_priority(),p)) );
      async_task(tsk);
    }
    
    /**
     *  This method will cancel all pending tasks causing them to throw cmt::error::thread_quit.
     *
     *  If the <i>current</i> thread is not <code>this</code> thread, then the <i>current</i> thread will
     *  wait for <code>this</code> thread to exit.
     *
     *  This is a blocking wait via <code>boost::thread::join</code>
     *  and other tasks in the <i>current</i> thread will not run while 
     *  waiting for <code>this</code> thread to quit.
     *
     *  @todo make quit non-blocking of the calling thread by eliminating the call to <code>boost::thread::join</code>
     */
    void quit();

    /**
     *  Process tasks until thread::quit() is called.
     */
    void exec();
    
    /**
     *  @return true unless quit() has been called.
     */
    bool is_running()const;

    priority current_priority()const;
    ~thread();

    protected:
      void set_boost_thread( boost::thread* t );
      friend class thread_private;
      friend void mace::cmt::yield();
      friend void mace::cmt::usleep( uint64_t );
      friend void mace::cmt::sleep_until( const system_clock::time_point& );

      // these methods may only be called from the current thread
      void yield();
      void usleep( uint64_t us );
      void sleep_until( const system_clock::time_point& tp );


      void wait( const promise_base::ptr& p, const microseconds& timeout_us );
      void wait( const promise_base::ptr& p, const system_clock::time_point& timeout );
      void notify( const promise_base::ptr& p );

    private:
      thread();

      friend class promise_base;
      void async_task( const task::ptr& t );
      class thread_private* my;
   };

   template<typename Functor>
   auto async( Functor&& f, const char* n, priority prio=priority()) -> cmt::future<decltype(f())> {
    return cmt::thread::current().async(std::forward<Functor>(f),(std::max)(current_priority(),prio),n);
   }
   template<typename Functor>
   auto async( Functor&& f, priority prio=priority(), const char* n = "") -> cmt::future<decltype(f())> {
    return cmt::thread::current().async(std::forward<Functor>(f),(std::max)(current_priority(),prio),n);
   }
   void async( const boost::function<void()>& t, priority prio=priority() ); 

   /**
  *   @brief Same as <code>cmt::current().exec()</code>
  */
   int  exec();

   /** 
  *  @brief Same as <code>cmt::current().usleep()</code>
  */
   inline void usleep( uint64_t us ) {
    mace::cmt::thread::current().usleep(us);
   }
   /** 
  *  @brief Same as <code>cmt::current().sleep_until()</code>
  */
   inline void sleep_until( const system_clock::time_point& tp ) {
    mace::cmt::thread::current().sleep_until(tp);
   }
   /** 
  *  @brief Same as <code>cmt::current().yield()</code>
  */
   void yield();
} } // mace::cmt

#endif // MACE_CMT_THREAD_HPP
