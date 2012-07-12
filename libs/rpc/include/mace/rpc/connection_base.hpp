#ifndef _MACE_RPC_CONNECTION_BASE_HPP_
#define _MACE_RPC_CONNECTION_BASE_HPP_
#include <boost/function.hpp>
#include <mace/rpc/message.hpp>
#include <mace/rpc/detail/pending_result.hpp>

#include <boost/signals.hpp>

namespace mace { namespace rpc  {

  typedef boost::function<message(message&)> method;

  namespace detail { class connection_base; }
  
  /**
   *  Defines the basic functionality for all RPC types.  This includes
   *  maintaining a database of methods that may be called and a pending
   *  result queue. 
   *
   *  Derived classes will implement the message packing/unpacking
   *  and transport method.
   */
  class connection_base : public boost::noncopyable {
    public:
      ~connection_base();

      /**
       *  @return null if no method with ID known.
       */
      const method* get_method( const std::string& name )const;

      /**
       *  Add a method with the given ID
       */
      void          add_method( const std::string& name, const method& );

      /**
       *  Add a method and generate an ID
       */
      std::string   add_method( const method& m );

      /**
       *  raw interface, returns the serialized data given the 
       *              serialized parameters.
       *
       *  @param mid method name to call
       */
      cmt::future<datavec> raw_call( std::string&& mid, datavec&& param );

      std::string create_method_id();

      connection_base& operator=( connection_base&& m ) {
        std::swap(m.my,my);
        return *this;
      }

      void close();

      /**
       *  Emited when read loop exits.
       */
      boost::signal<void()> closed;
    protected:
      /**
       *  Call method ID with param and use the given result handler if specified.  
       *
       *  @param pr - result handler, if not specified result will be ignored.
       */
      void raw_call( std::string&& meth, datavec&& param, 
                     const detail::pending_result::ptr& pr );

      connection_base( detail::connection_base* b );
      connection_base( connection_base&& m ):my(m.my) { m.my=0; }

      detail::connection_base* my;
    private:
      connection_base();
  };


} }  // namespace mace::rpc

#endif // _MACE_RPC_CONNECTION_BASE_HPP_
