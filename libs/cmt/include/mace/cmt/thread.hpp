/**
 * @file mace/cmt/thread.hpp
 */
#ifndef MACE_CMT_THREAD_HPP
#define MACE_CMT_THREAD_HPP
#include <mace/cmt/task.hpp>
#include <boost/chrono.hpp>
#include <mace/cmt/abstract_thread.hpp>

namespace boost { class thread; }

namespace mace { 
/**
 *  @brief All types that are part of the MACE Cooperative Multi-Tasking Library
 */
namespace cmt {
   using boost::chrono::microseconds;
   using boost::chrono::system_clock;

   struct context;

   priority    current_priority();
   inline void usleep( uint64_t us );
   inline void sleep_until( const system_clock::time_point& tp );

   system_clock::time_point to_time_point( const boost::posix_time::ptime& from );

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
    bool           is_current()const;

    /**
     *  @brief returns the name given by @ref set_name() for this thread
     */
    const char* name()const;

    /**
     *  @brief associates a name with this thread.
     */
    void    set_name( const char* n );

    /**
     *  @brief print debug info about the state of every context / promise.
     *
     *  This method is helpful to figure out where your program is 'hung' by listing
     *  every async operation (context) and what it is blocked on (future).  
     *
     *  @note debug info is more useful if you provide a description for your
     *  async tasks and promises.
     */
    void    debug( const std::string& d );

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
                   priority prio = priority() ) -> future<decltype(f())> {
         typedef decltype(f()) Result;
         typename promise<Result>::ptr p(new promise<Result>());
         task::ptr tsk( new rtask<Functor,Result>( std::forward<Functor>(f),p,when,std::max(current_priority(),prio)) );
         p->set_task(tsk);
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
    auto async( Functor&& f, priority prio = priority()) -> future<decltype(f())> {
       typedef decltype(f()) Result;
       typename promise<Result>::ptr p(new promise<Result>());
       task::ptr tsk( new rtask<Functor,Result>( std::forward<Functor>(f),p,std::max(current_priority(),prio)) );
       p->set_task(tsk);
       async_task(tsk);
       return p;
    }

    /**
     *  Calls function <code>f</code> in this thread and returns a future<T> that can
     *  be used to wait on the result.  
     *
     *  @see debug
     *
     *  @param f the operation to perform
     *  @param desc a description that shows up in debug messages.
     *  @param prio the priority relative to other tasks
     *  @param n a debut name to associate with this task.
     */
    template<typename Functor>
    auto async( Functor&& f, const char* desc, priority prio = priority()) -> future<decltype(f())> {
       typedef decltype(f()) Result;
       typename promise<Result>::ptr p(new promise<Result>(desc));
       task::ptr tsk( new rtask<Functor,Result>( std::forward<Functor>(f),p,std::max(current_priority(),prio)) );
       tsk->set_desc(desc);
       p->set_task(tsk);
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
                        priority prio = priority()) {
      task::ptr tsk( new vtask<Functor>(std::forward<Functor>(f),when,std::max(current_priority(),prio)) );
      async_task(tsk);
    }
    template<typename Functor>
    void post( Functor&& f, const system_clock::time_point& when, const char* desc = "",
                        priority prio = priority()) {
      task::ptr tsk( new vtask<Functor>(std::forward<Functor>(f),when,std::max(current_priority(),prio)) );
      tsk->set_desc(desc);
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
     *  Calls function <code>f</code> in this thread. Returns immediately.
     *
     *  @param p - the priority of a task.
     *  @param t - the task to be run.
     */
    template<typename Functor>
    void post( Functor&& f, const char* desc, priority p = priority() ) {
      task::ptr tsk( new vtask<Functor>(std::forward<Functor>(f),std::max(current_priority(),p)) );
      tsk->set_desc( desc );
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


   template<typename T1, typename T2>
   static int wait_any( mace::cmt::future<T1>& f1, mace::cmt::future<T2>& f2, const microseconds& timeout_us = microseconds::max() );

    private:
      void set_boost_thread( boost::thread* t );
      friend class thread_private;
      friend class mutex;


      friend void mace::cmt::yield();
      friend void mace::cmt::usleep( uint64_t );
      friend void mace::cmt::sleep_until( const system_clock::time_point& );

      // these methods may only be called from the current thread
      void yield( bool reschedule = true );
      void yield_until( const system_clock::time_point& tp, bool reschedule = true );
      void usleep( uint64_t us );
      void sleep_until( const system_clock::time_point& tp );

      void wait( const promise_base::ptr& p, const microseconds& timeout_us );
      void wait( const promise_base::ptr& p, const system_clock::time_point& timeout );

      int wait_any( const std::vector<promise_base::ptr>& p, const microseconds& timeout_us );
      int wait_any( const std::vector<promise_base::ptr>& p, const system_clock::time_point& timeout );
      void notify( const promise_base::ptr& p );

      cmt::context* current_context()const;

      void          unblock( cmt::context* c );

    private:
      thread();

      friend class promise_base;
      void async_task( const task::ptr& t );
      class thread_private* my;
   };

   /**
    * Wait until either f1 or f2 is ready.
    *
    * @return 0 if f1 is ready, 1 if f2 is ready or throw on error.
    */
   template<typename T1, typename T2>
   int wait_any( mace::cmt::future<T1>& f1, mace::cmt::future<T2>& f2, const microseconds& timeout_us = microseconds::max() ) {
     return mace::cmt::thread::wait_any(f1,f2);
   }
   template<typename T1, typename T2>
   int thread::wait_any( mace::cmt::future<T1>& f1, mace::cmt::future<T2>& f2, const microseconds& timeout_us ) {
     std::vector<promise_base::ptr> p(2);
     p[0] = f1.m_prom;
     p[1] = f2.m_prom;
     return cmt::thread::current().wait_any(p,timeout_us);
   }

   template<typename Functor>
   auto async( Functor&& f, priority prio=priority()) -> cmt::future<decltype(f())> {
    return cmt::thread::current().async(std::forward<Functor>(f),(std::max)(current_priority(),prio));
   }
   template<typename Functor>
   auto async( Functor&& f, const char* desc, priority prio=priority()) -> cmt::future<decltype(f())> {
    return cmt::thread::current().async(std::forward<Functor>(f),desc,(std::max)(current_priority(),prio));
   }
   void async( const boost::function<void()>& t, priority prio=priority() ); 
   void async( const boost::function<void()>& t, const char* desc, priority prio=priority() ); 

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
  *  @brief Same as <code>cmt::thread::current().yield()</code>
  */
   void yield();

   /// short for cmt::thread::current().quit()
   inline void quit() { cmt::thread::current().quit(); }
} } // mace::cmt

#endif // MACE_CMT_THREAD_HPP
