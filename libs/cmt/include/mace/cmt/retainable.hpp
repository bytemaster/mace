#ifndef _MACE_CMT_RETAINABLE_HPP_
#define _MACE_CMT_RETAINABLE_HPP_
#include <boost/assert.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/atomic.hpp>
#include <boost/memory_order.hpp>

namespace mace { namespace cmt {

/**
 *  Allows an instance of retainable type T to be allocated on
 *  the stack and freed by normal means.
 */
template<typename T>
class stack_retainable : public T {
    public:
        template<typename A, typename B, typename C>
        stack_retainable( const A& a, const B& b, const C& c)
        :T(a,b,c){}
        template<typename A, typename B, typename C, typename D>
        stack_retainable( const A& a, const B& b, const C& c, const D& d)
        :T(a,b,c,d){}

        stack_retainable(){}
        ~stack_retainable() {}
};

/**
    @brief provides reference counted type

    Use with retainable_ptr<T> to get automatic retain/release functionality.

    This class uses atomic operations so the reference count is
    safe for sharing objects among threads
*/
class retainable {
    public:
        template<typename T>
        friend class stack_retainable;

        retainable():m_ref_count(1) {}

        inline void retain() {
            m_ref_count.fetch_add(1, boost::memory_order_relaxed);
        }

        inline void release() {
            if( 1 == m_ref_count.fetch_sub(1, boost::memory_order_release) ) {
                boost::atomic_thread_fence(boost::memory_order_acquire);
                delete this;
            }
        }

    protected:
        virtual ~retainable() {}

    private:
        retainable(const retainable&):m_ref_count(1) {}

        retainable& operator=(const retainable& ) {
            return *this;
        }

        boost::atomic<int32_t>  m_ref_count;
};




template<typename T>
class retainable_ptr
{
    public:
        explicit retainable_ptr(T* t = 0, bool inc = false)
        :cnt(t) {
            if( inc && cnt ) cnt->retain();
        }

        retainable_ptr( const retainable_ptr<T>& copy )
        :cnt(copy.cnt) {
            if( cnt ) cnt->retain();
        }

#ifdef BOOST_HAS_RVALUE_REFS
        retainable_ptr( retainable_ptr<T>&& mv )
        :cnt(mv.cnt) {
            mv.cnt = NULL;
        }

        inline retainable_ptr<T>& operator=(retainable_ptr<T>&& mv ) {
            cnt = mv.cnt;
            mv.cnt = NULL;
            return *this;
        }
#endif
        ~retainable_ptr() {
            if( cnt ) cnt->release();
        }

        template<typename U>
        operator retainable_ptr<U>()const {
            retainable_ptr<U> u;
            u.cnt = dynamic_cast<U*>(cnt);
            if( u.cnt ) 
                u.cnt->retain();
            return u;
        }
        retainable_ptr& reset()  {
            if( !cnt ) 
                return *this; 
            cnt->release();
            cnt = 0; 
            return *this; 
        }

        inline bool operator!()const { return cnt == 0; }
        inline operator bool()const { return cnt != 0; }

        inline retainable_ptr<T>& operator=(const retainable_ptr<T>& copy ) {
            if( cnt == copy.cnt ) 
                return *this;
            if( cnt ) 
                cnt->release();
            cnt = copy.cnt;
            if( cnt ) 
                cnt->retain();
            return *this;
        }
        inline T& operator* () const  { return *cnt; }
        inline T * operator-> () const { return cnt; }

        inline bool operator==( const retainable_ptr& p )const {
            return get() == p.get();
        }
        inline bool operator<( const retainable_ptr& p )const {
            return get() < p.get();
        }
        inline T * get() const { return cnt; }

    private:
        template<typename U> friend class retainable_ptr;
        T* cnt;
};
template<typename T>
T* get_pointer( const retainable_ptr<T>& p ) { return p.get(); }

template<class T, class U> 
inline bool operator==(retainable_ptr<T> const & a, retainable_ptr<U> const & b) {
    return a.get() == b.get();
}

template<class T, class U> 
inline bool operator!=(retainable_ptr<T> const & a, retainable_ptr<U> const & b) {
    return a.get() != b.get();
}

template<class T, class U> 
inline retainable_ptr<T> dynamic_retainable_cast( const retainable_ptr<U>& u ) {
    T* t = dynamic_cast<T*>(u.get());
    if( t ) t->retain();
    return retainable_ptr<T>( t );
}
template<class T, class U> 
inline retainable_ptr<T> static_retainable_cast( const retainable_ptr<U>& u ) {
    T* t = static_cast<T*>(u.get());
    if( t )
        t->retain();
    return retainable_ptr<T>( t );
}

} } // namespace mace::cmt

#endif
