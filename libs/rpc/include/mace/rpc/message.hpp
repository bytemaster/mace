#ifndef _MACE_RPC_MESSAGE_HPP_
#define _MACE_RPC_MESSAGE_HPP_
#include <vector>
#include <mace/rpc/varint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <utility>

namespace mace { namespace rpc {

  typedef std::vector<char> datavec;

  struct error_object {
    error_object( int c, const std::string& s )
    :code(c),message(s){};
    error_object( int c = 0):code(c){}
    signed_int                     code;
    boost::optional<std::string>   message;
  };

  /**
   *  Generic message used to pass parameters across the network.
   */
  class message {
    public:
    typedef std::string       method_id_type;
    typedef uint32_t          request_id_type;
    typedef std::vector<char> param_type;  
    typedef param_type        result_type;  
    typedef error_object      error_type;  

    struct id_factory {
      public:
         id_factory():next_request(0),next_method(0){}

         request_id_type create_request_id() {
          return next_request++;
         }
         method_id_type create_method_id() {
          return boost::lexical_cast<method_id_type>(next_method++);
         }
      private:
         request_id_type next_request;
         int64_t         next_method;             
    };


    bool has_request_id()const { return !!id;     }
    bool has_method_id()const  { return !!method; }
    bool has_error()const      { return !!error;  }
    bool has_params()const     { return !!params; }
    bool has_result()const     { return !!result; }
    
    /// @pre has_request_id()
    const request_id_type& get_request_id()const { return *id;     }
    /// @pre has_method_id()
    const method_id_type& get_method_id()const   { return *method; }
    /// @pre has_params()
    const param_type& get_params()const          { return *params; }
    /// @pre has_error()
    const error_type& get_error()const           { return *error;  }
    /// @pre has_result()
    const result_type& get_result()const         { return *result; }


    request_id_type&& take_request_id(){ return std::move(*id);     }
    /// @pre has_method_id()
    method_id_type&& take_method_id()  { return std::move(*method); }
    /// @pre has_params()
    param_type&& take_params()         { return std::move(*params); }
    /// @pre has_error()
    error_type&& take_error()          { return std::move(*error);  }
    /// @pre has_result()
    result_type&& take_result()        { return std::move(*result); }

    void set_request_id( request_id_type&& rid ) {
       // until boost::optional supports move, this is what we have to do
      if( !id ) id = request_id_type(); 
      *id = std::move(rid);
    }
    void set_method_id( method_id_type&& mid ) {
      if( !method ) method = method_id_type();
      *method = std::move(mid);
    }
    void set_result( result_type&& r ) {
      if( !result ) result = result_type();
      *result = std::move(r);
    }
    void set_error( error_type&& r ) {
      if( !error ) error = error_type();
      *error = std::move(r);
    }
    void set_params( param_type&& r ) {
      if( !params ) params = param_type();
      *params = std::move(r);
    }
    message(){}

    explicit message( const message& c )
    :id(c.id),error(c.error),method(c.method),params(c.params),result(c.result){}

    message( message&& m ) {
      *this = std::move(m);
    }
    message& operator=( message&& m ) {
      if( m.has_request_id() ){ set_request_id( m.take_request_id() ); }
      if( m.has_method_id() ) { set_method_id( m.take_method_id() );   }
      if( m.has_error() )     { set_error( m.take_error() );           }
      if( m.has_params() )    { set_params( m.take_params() );         }
      if( m.has_result() )    { set_result( m.take_result() );         }
      return *this;
    }

    message( method_id_type&& mid, param_type&& param, const request_id_type& rid )
    :id(rid),method(method_id_type()),params(param_type()) {
      *method = std::move(mid);
      *params = std::move(param);
    }
    message( method_id_type&& mid, param_type&& param )
    :method(method_id_type()),params(param_type()) {
      *method = std::move(mid);
      *params = std::move(param);
    }
    message( request_id_type&& rid, result_type&& r )
    :id(request_id_type()),result(result_type()) {
      *result = std::move(r);
    }
    message( request_id_type&& rid, error_type&& e )
    :id(request_id_type()),error(error_type()) {
      *error = std::move(e);
    }

    private: // this implementation could change
       boost::optional<request_id_type>  id;     
       boost::optional<error_type>       error;  
       boost::optional<method_id_type>   method; 
       boost::optional<param_type>       params; 
       boost::optional<result_type>      result;
  };

} }

#endif // _MACE_RPC_MESSAGE_HPP_
