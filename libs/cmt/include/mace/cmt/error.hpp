#ifndef _MACE_CMT_ERROR_HPP_
#define _MACE_CMT_ERROR_HPP_
#include <boost/exception/all.hpp>
#include <boost/format.hpp>

namespace mace { namespace cmt {
    namespace error {
        struct future_exception : public std::exception, public virtual boost::exception {
            const char*  what()const throw() { return "future exception";     }
            virtual void rethrow()const      { BOOST_THROW_EXCEPTION(*this);  }
        };

        struct null_future : public virtual future_exception {
            const char*  what()const throw() { return "attempt to access null future"; }
            virtual void rethrow()const      { BOOST_THROW_EXCEPTION(*this); }
        };

        struct future_value_not_ready : public virtual future_exception {
            const char*  what()const throw() { return "the future value is not ready"; }
            virtual void rethrow()const      { BOOST_THROW_EXCEPTION(*this); }
        };

        struct future_wait_timeout : public virtual future_exception {
            const char*  what()const throw() { return "timeout waiting for future"; }
            virtual void rethrow()const      { BOOST_THROW_EXCEPTION(*this); }
        };
        struct wait_any_error : public virtual future_exception {
            const char*  what()const throw() { return "none of the futures were ready"; }
            virtual void rethrow()const      { BOOST_THROW_EXCEPTION(*this); }
        };
        struct broken_promise : public virtual future_exception {
            const char*  what()const throw() { return "broken promise - operation timeout"; }
            virtual void rethrow()const      { BOOST_THROW_EXCEPTION(*this); }
        };
        struct task_canceled : public virtual future_exception {
            const char*  what()const throw() { return "task canceled";       }
            virtual void rethrow()const      { BOOST_THROW_EXCEPTION(*this); }
        };
        struct thread_quit : public virtual future_exception {
            const char*  what()const throw() { return "thread quit"; }
            virtual void rethrow()const      { BOOST_THROW_EXCEPTION(*this); }
        };
        struct null_rvalue : public virtual future_exception {
            const char*  what()const throw() { return "null rvalue"; }
            virtual void rethrow()const      { BOOST_THROW_EXCEPTION(*this); }
        };
    } // namespace error
    typedef boost::error_info<struct err_msg_,std::string> err_msg;

    struct exception : public virtual boost::exception, public virtual std::exception {
        const char* what()const throw()     { return "exception";                     }
        virtual void       rethrow()const   { BOOST_THROW_EXCEPTION(*this);                  } 
        const std::string& message()const   { return *boost::get_error_info<mace::cmt::err_msg>(*this); }
    };
} } // namespace mace::cmt

#define MACE_CMT_THROW( MSG, ... ) \
  do { \
    BOOST_THROW_EXCEPTION( mace::cmt::exception() << mace::cmt::err_msg( (boost::format( MSG ) __VA_ARGS__ ).str() ) );\
  } while(0)

#endif // _MACE_CMT_ERROR_HPP_
