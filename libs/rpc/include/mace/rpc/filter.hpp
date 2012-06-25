#ifndef _MACE_RPC_FILTER_HPP_
#define _MACE_RPC_FILTER_HPP_
#include <boost/function.hpp>
#include <mace/void.hpp>
#include <boost/fusion/container/vector.hpp>
namespace mace { namespace rpc {

    /**
     *  @brief performs no transformations.
     *
     *  Implements the bare minimum Filter concept.
     */
    struct default_filter {
      /**
       *  Given input T convert and assign to output v.
       */
      template<typename T>
      void operator()( const T& r, T& v )const  { v = r; }

      /**
       *  @return true if filtered for a particular type.
       */
      template<typename T>
      const bool is_filtered(const T*)const { return false; }

      /**
       *  Pack filter, given input type return output type.
       */
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
    struct function_filter {
      function_filter( Connection& c ):m_con(c){}

      /**
       *  Given input T convert and assign to output v.
       */
      template<typename T>
      void operator()( const T& r, T& v )const  { v = r; }
      
      template<typename Signature>
      void operator()( const std::string& r, boost::function<Signature>& v ) {
        v = m_con.template create_callback<Signature>(r);
      }
      
      template<typename Signature>
      const bool is_filtered(boost::function<Signature>*)const { return true; }
      template<typename Signature>
      const bool is_filtered(std::function<Signature>*)const { return true; }

      /**
       *  @return true if filtered for a particular type.
       */
      template<typename T>
      const bool is_filtered(const T*)const { return false; }

      /**
       *  Pack filter, given input type return output type.
       *   @todo only 'copy' rvalues, hopefully a smart optimizer will avoid a copy
       *         with this implementation, but we make it more explicit that we don't 
       *         want to copy const T& v to the return value....
       */
      template<typename T>
      inline auto operator()( const T& v )const -> T {
        return v; 
      }
      template<typename T>
      inline auto operator()( const boost::function<T>& v )const -> std::string {
        return  m_con.add_method(v);
      }

      private:
        Connection& m_con;
    };

    template<>
    struct function_filter<void>  {
      /**
       *  Given input T convert and assign to output v.
       */
      template<typename T>
      void operator()( const T& r, T& v )const  { v = r; }
      
      template<typename Signature>
      void operator()( const std::string& r, boost::function<Signature>& v ) {
      }
      
      template<typename Signature>
      const bool is_filtered(boost::function<Signature>*)const { return true; }
      template<typename Signature>
      const bool is_filtered(std::function<Signature>*)const { return true; }

      /**
       *  @return true if filtered for a particular type.
       */
      template<typename T>
      const bool is_filtered(const T*)const { return false; }

      /**
       *  Pack filter, given input type return output type.
       *   
       *   @todo only 'copy' rvalues, hopefully a smart optimizer will avoid a copy
       *         with this implementation, but we make it more explicit that we don't 
       *         want to copy const T& v to the return value....
       */
      template<typename T>
      inline auto operator()( const T& v )const -> T {
        return v; 
      }
      template<typename T>
      inline auto operator()( const boost::function<T>& v )const -> mace::void_t {
        return mace::void_t();
      }
    };

} } // namespace boost::rpc

#endif //_MACE_RPC_FILTER_HPP_
