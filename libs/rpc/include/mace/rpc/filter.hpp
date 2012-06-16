#ifndef _MACE_RPC_FILTER_HPP_
#define _MACE_RPC_FILTER_HPP_
#include <boost/function.hpp>
namespace mace { namespace rpc {

    /**
     *  @brief performs no transformations.
     *
     *  Implements the bare minimum Filter concept.
     */
    struct default_filter {
      template<typename T>
      void operator()( const T& r, T& v )const  { v = r; }

      template<typename T>
      const bool is_filtered(const T*)const { return false; }

      template<typename T>
      inline const T& operator()( const T& v )const { return v; }


      template<typename T>
      inline T operator()( T&& v )const { return v; }
    };

    /**
     *  @tparam Connection must implement the following expressions:
     *
     *    R                          Connection::add_method( const boost::function<Signature>& ); 
     *    boost::function<Signature> Connection::create_callback<Signature>( const R& );
     *
     */
    template<typename Connection>
    struct function_filter : default_filter {
      function_filter( Connection& c ):m_con(c){}

      using default_filter::operator();
      using default_filter::is_filtered;

      template<typename Signature>
      auto operator()( const boost::function<Signature>& v ) -> decltype( ((Connection*)0)->add_method(v)) {
        return m_con.add_method( v );
      }
      
      template<typename Signature, typename R>
      void operator()( const R& r, boost::function<Signature>& v ) {
        v = m_con.template create_callback<Signature>(r);
      }
      
      template<typename Signature>
      const bool is_filtered(const boost::function<Signature>*)const { return true; }
      private:
        Connection& m_con;
    };

} } // namespace boost::rpc

#endif //_MACE_RPC_FILTER_HPP_
