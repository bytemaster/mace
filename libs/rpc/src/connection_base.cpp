#include <mace/rpc/connection_base.hpp>
#include <mace/rpc/detail/connection_base.hpp>
#include <utility>

namespace mace { namespace rpc {
  
  connection_base::connection_base( detail::connection_base* b ):my(b){}
  connection_base::~connection_base() {
    delete my;
  }

  const method* connection_base::get_method( const std::string& n )const {
    detail::method_map::const_iterator i = my->methods.find(n);
    if( i != my->methods.end() ) return &i->second;
    return 0; 
  }

  void connection_base::add_method( const std::string& name, const method& m ) {
    my->methods[name] = m;
  }

  std::string connection_base::add_method( const method& m ) {
    std::string nn = create_method_id();
    add_method( nn, m );
    return nn;
  }

  std::string connection_base::create_method_id() {
    return "mace_rpc_cb" + boost::lexical_cast<std::string>(++my->next_method_id);
  }

  cmt::future<datavec> connection_base::raw_call( std::string&& mid, datavec&& param ) {
    mace::cmt::promise<datavec>::ptr prom(new mace::cmt::promise<datavec>() );
    detail::pending_result::ptr pr( new detail::raw_pending_result(prom) );
    raw_call( std::move(mid), std::move(param), pr );
    return prom;
  }

  void connection_base::raw_call( std::string&& meth, datavec&& param, const detail::pending_result::ptr& pr ) {
    if( pr ) {
        my->send( message( std::move(meth), std::move(param), ++my->next_req_id ) );
        my->results[my->next_req_id] = pr;
    } else 
        my->send( message( std::move(meth), std::move(param) ) );
  }

} } // namepace mace::rpc
