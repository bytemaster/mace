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

namespace mace { namespace cmt {
    using namespace boost::chrono;

     struct cmt_context;

     struct priority {
       explicit priority( int v = 0):value(v){}
       priority( const priority& p ):value(p.value){}
       bool operator < ( const priority& p )const {
        return value < p.value;
       }
       int value;
     };
    class task : public retainable {
        public:
            typedef task* ptr;
            task(priority p=priority(),const system_clock::time_point& w = system_clock::time_point::min() )
            :when(w),prio(p),next(0),active_context(0),canceled(false){
                static int64_t global_task_count=0;
                posted_num = ++global_task_count;
            }
            ~task() { }

            virtual void run() = 0;
            /// implemented in thread.cpp
            virtual void cancel();
            virtual const char* name() { return "unknown"; }
        protected:
            void                  set_active_context( cmt_context* c ) {
              boost::unique_lock<cmt::spin_lock> lock( active_context_lock );
              active_context = c;
            }
            static int64_t        task_num;
            friend class          thread;
            friend class          thread_private;

            system_clock::time_point when;
            bool                                    canceled;
            uint64_t                                posted_num;
            priority                                prio;
            task*                                   next;
            cmt_context*                            active_context;
            mace::cmt::spin_lock                    active_context_lock;
    };

    template<typename R = void>
    class rtask : public task {
        public:
            rtask( const boost::function<R()>& f, const typename promise<R>::ptr& p, const system_clock::time_point& tp, priority prio, const char* name = "" )
            :task(prio,tp),m_functor(f),m_prom(p),m_name(name){ m_prom->set_task(this); }

            rtask( const boost::function<R()>& f, const typename promise<R>::ptr& p, priority prio, const char* name = "" )
            :task(prio),m_functor(f),m_prom(p),m_name(name){ m_prom->set_task(this); }
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
            const char* name() { return m_name; }

            boost::function<R()>        m_functor;
            typename promise<R>::ptr    m_prom;
            const char*                 m_name;
    };

    template<>
    class rtask<void> : public task {
        public:
            rtask( const boost::function<void()>& f, const promise<void>::ptr& p, const system_clock::time_point& tp, priority prio, const char* name = "" )
            :task(prio,tp),m_functor(f),m_prom(p),m_name(name){ m_prom->set_task(this); }

            rtask( const boost::function<void()>& f, const  promise<void>::ptr& p, priority prio=priority(), const char* name = "" )
            :task(prio),m_functor(f),m_prom(p),m_name(name){ m_prom->set_task(this); }
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
            const char* name() { return m_name; }
        
            boost::function<void()>     m_functor;
            promise<void>::             ptr m_prom;
            const char*                 m_name;

    };


    class vtask : public task {
        
        public:
        vtask( const boost::function<void()>& f, priority prio = priority(), const char* n="" )
        :task(prio),m_functor(f),m_name(n){ }
        vtask( const boost::function<void()>& f, const system_clock::time_point& when, priority prio = priority(), const char* n="" )
        :task(prio,when),m_functor(f),m_name(n){ }

        void cancel() {}
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
        virtual const char* name() { return m_name; }
        boost::function<void()> m_functor;
        const char*             m_name;
    };

} } // namespace mace::cmt

#endif // MACE_CMT_TASK_HPP
