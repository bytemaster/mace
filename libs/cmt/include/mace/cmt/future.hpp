#ifndef _MACE_CMT_FUTURE_HPP
#define _MACE_CMT_FUTURE_HPP
#include <mace/cmt/retainable.hpp>
#include <mace/cmt/error.hpp>
#include <mace/cmt/log/log.hpp>
#include <boost/thread/mutex.hpp>
#include <mace/cmt/mutex.hpp>
#include <boost/optional.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/condition_variable.hpp>

namespace mace { namespace cmt {
    using boost::chrono::microseconds;
    using boost::chrono::system_clock;
    boost::system_time to_system_time( const boost::chrono::system_clock::time_point& t );

    class abstract_thread;
    class task;
    class promise_base :  public retainable {
         public:
             typedef retainable_ptr<promise_base> ptr;
             promise_base():m_task(0),m_blocked_thread(0),m_timeout(microseconds::max()){}
             virtual ~promise_base(){}

             void         set_task( task* t );
             void         cancel();
             virtual bool ready()const = 0;
         protected:
             void enqueue_thread();
             void wait( const microseconds& timeout_us );
             void wait_until( const system_clock::time_point& timeout_us );
             void notify();
             virtual void set_timeout()=0;
             virtual void set_exception( const boost::exception_ptr& e )=0;

         private:
             friend class thread;
             friend class thread_private;

             task*                     m_task;
             abstract_thread*          m_blocked_thread;
             microseconds              m_timeout;    
    };

    struct void_t {};

    /**
     *  This promise blocks cooperatively until the value is
     *  provided.  It will allow other tasks to run in the
     *  current thread if wait() is called while there is
     *  a current mace::cmt::thread stack.
     */
    template<typename T = void_t>
    class promise : public promise_base {
        public:
            typedef retainable_ptr<promise> ptr;

            promise(){}
            promise( const T& v ):m_value(v){}

            bool error()const { return m_error; }
            virtual bool ready()const { 
               boost::unique_lock<mutex> lock( m_mutex );
               return ( m_error || m_value ); 
            }

            virtual const T& wait(const microseconds& timeout = microseconds::max() ){
                { // lock while we check values
                    boost::unique_lock<mutex> lock( m_mutex );
                    if( m_error ) boost::rethrow_exception(m_error);
                    if( m_value ) return *m_value;
                    enqueue_thread();
                } // unlock before yielding, but after enqueing
                promise_base::wait(timeout);
                if( m_error ) { 
                  boost::exception_ptr    er = m_error;
                  m_error = boost::exception_ptr();
                  boost::rethrow_exception(er);
                }
                if( m_value ) return *m_value;
                BOOST_THROW_EXCEPTION( error::future_value_not_ready() ); 
                return *m_value;
            }

            virtual const T& wait_until(const system_clock::time_point& timeout  ){
                { // lock while we check values
                    boost::unique_lock<mutex> lock( m_mutex );
                    if( m_error ) boost::rethrow_exception(m_error);
                    if( m_value ) return *m_value;
                    enqueue_thread();
                } // unlock before yielding, but after enqueing
                promise_base::wait_until(timeout);
                if( m_error ) { 
                  boost::exception_ptr    er = m_error;
                  m_error = boost::exception_ptr();
                  boost::rethrow_exception(er);
                }
                if( m_value ) return *m_value;
                BOOST_THROW_EXCEPTION( error::future_value_not_ready() ); 
                return *m_value;
            }

            virtual void set_exception( const boost::exception_ptr& e ) {
                {
                    boost::unique_lock<mutex> lock( m_mutex );
                    m_error = e;
                }
                notify();
            }
            virtual void set_value( const T& v ) {
                {
                    boost::unique_lock<mutex> lock( m_mutex );
                    if( m_error ) 
                        return;
                    m_value = v;
                }
                notify();
            }
            
        protected:
            virtual void set_timeout() {
                {
                    boost::unique_lock<mutex> lock( m_mutex );
                    if( m_value ) 
                        return;
                    m_error = boost::copy_exception( error::future_wait_timeout() );
                }
                notify();
            }

            mutable cmt::mutex            m_mutex;
            mutable boost::exception_ptr  m_error;
            mutable boost::optional<T>    m_value;
    };


    template<>
    class promise<void> : public promise<void_t> {};


    /**
     * @brief placeholder for the result of an asynchronous operation.
     *  
     * A future is constructed with a promise that is created when a new asynchronous 
     * task is started.  A future behaves like a shared pointer where all copies
     * reffer to the same promise. 
     *
     * When future::wait() is called, the future will block the current task until one
     * of three conditions are met:
     *  - timeout
     *  - a value is set
     *  - an exception is set
     *
     * If the asynchronous operation threw an exception it will be rethrown from the
     * call to wait().
     *
     * @section future_auto_convert Automatic waiting on cast.
     *
     * mace::cmt::future<T> automatically casts to type T when requested.  This cast
     * is short hand for mace::cmt::future<T>::wait().
     *  
     */
    template<typename T = void_t>
    class future {
        public:
	        typedef typename promise<T>::ptr promise_ptr;
            typedef T                        value_type;

            future( const promise_ptr& p = promise_ptr() )
            :m_prom(p){}
            future( const T& v ):m_prom( new promise<T>(v) ){}
            
            void cancel()     { if( m_prom && !ready() ) m_prom->cancel(); }

            bool valid()const { return !!m_prom;       }
            bool ready()const { return m_prom->ready();}
            bool error()const { return valid() ? m_prom->error() : false; }
            operator const T&()const { 
                if( !m_prom ) BOOST_THROW_EXCEPTION( error::null_future() );
                return m_prom->wait();
            }
            const T& wait(const microseconds& timeout = microseconds::max() )const { 
                if( !m_prom ) BOOST_THROW_EXCEPTION( error::null_future() );
                return m_prom->wait(timeout); 
            }
            const T& wait_until(const system_clock::time_point& timeout )const { 
                if( !m_prom ) BOOST_THROW_EXCEPTION( error::null_future() );
                return m_prom->wait_until(timeout); 
            }


        private:
            mutable promise_ptr m_prom;
    };

    template<>
    class future<void> : public future<void_t> {
        public:
            future( const  promise<void_t>::ptr& p =  promise<void_t>::ptr() )
            :future<void_t>(p){}
            future( const void_t& v ):future<void_t>(v){}
    };


} } // mace::cmt


#endif
